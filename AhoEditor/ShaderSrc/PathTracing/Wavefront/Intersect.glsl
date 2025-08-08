#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable

#define PATH_TRACING
#define OPT_SHADOW_TEST

// #define SHARED_WRITE
// #define INDIRECT_DISPATCH

#include "../PathTracingCommon.glsl"
#include "../IntersectionTest.glsl"
#include "../LightSources/DirectLight.glsl"
#include "../DisneyPrincipled.glsl"

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;

uniform int u_Frame = 1;
uniform int u_MaxBounce = 8;
uniform int u_WriteIndex;
uniform int u_ReadIndex;
uniform int u_SrcWidth;
uniform int u_SrcHeight;

float MaxComponents(vec3 v) {
    return max(v.x, max(v.y, v.z));
}

shared uint localCount;
shared uint localIndices[64]; // 假设local_size_x = 64
shared Payload localPayloads[64];

#ifdef INDIRECT_DISPATCH
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
#else
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
#endif

void main() {
#ifdef INDIRECT_DISPATCH
    int globalIdx = int(gl_GlobalInvocationID.x);
    // --- Consume a payload from queue ---
    Payload payload;
	if (u_ReadIndex == 0) {
		payload = s_Payload0[globalIdx];
        s_Payload0[globalIdx].alive = false;
    }
    else {
		payload = s_Payload1[globalIdx];
        s_Payload1[globalIdx].alive = false;
    }

    if (!payload.alive || payload.bounce > u_MaxBounce)
        return; // Should not happen, but just in case

	uint pixelIndex = payload.pixelIndex;
    ivec2 coords = ivec2(pixelIndex % u_SrcWidth, pixelIndex / u_SrcWidth);
	if (coords.x >= u_SrcWidth || coords.y >= u_SrcHeight)
		return; // Should not happen, but just in case

#else
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= u_SrcWidth || coords.y >= u_SrcHeight)
		return; 

    uint globalIdx = coords.y * u_SrcWidth + coords.x;
    Payload payload;
	if (u_ReadIndex == 0) {
		payload = s_Payload0[globalIdx];
        s_Payload0[globalIdx].alive = false;
    }
    else {
		payload = s_Payload1[globalIdx];
        s_Payload1[globalIdx].alive = false;
    }
    if (!payload.alive || payload.bounce > u_MaxBounce)
        return; 
#endif

    // --- Initialize RNG for this bounce ---
    InitRNG(vec2(coords), int(u_Frame * u_MaxBounce + payload.bounce));

    // --- Intersect scene ---
    Ray ray;
    ray.origin = payload.origin;
    ray.direction = payload.direction;

    HitInfo info = InitHitInfo();
    RayTrace(ray, info);


    // return;

    // --- State from last bounce ---
    vec3 beta = payload.throughput;
    vec3 L = vec3(0);
    float pdf = payload.pdf;
    
    State state;
    state.cosTheta = payload.cosTheta;
    state.eta = payload.eta;
    state.N = payload.N;
    state.pos = payload.pos;

    // --- Miss ray: Evaluate infinite light ---
    if (!info.hit) {
        vec4 Le_pdf = EvaluateInfiniteLight(ray);
        vec3 Le = Le_pdf.rgb;
        if (Le_pdf.a <= 0.0 || payload.bounce == 1) {
            L += beta * Le;
        } else {
            float misWeight = PowerHeuristicPdf(pdf, Le_pdf.a);
            L += misWeight * beta * Le;
        }
        payload.alive = false;
    } 
    // --- Hit ray: bsdf sampling and evaluation ---
    else {
        PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
        RetrievePrimInfo(state, p, info.uv); // Get material info
        
        L += beta * SampleDirectLight(state, ray); 

        state.cosTheta = dot(state.N, -ray.direction); // cosTheta changed during IBL sampling, so set it again

        // --- Sample next ray ---
        vec3 Lworld;
        float new_bsdf_pdf;
        vec3 f = Sample(state, -ray.direction, Lworld, new_bsdf_pdf);
        if (new_bsdf_pdf > 0.0 && payload.bounce < u_MaxBounce && MaxComponents(beta) >= 0.005) {
            beta *= f / new_bsdf_pdf;

            // --- Russian roulette
            float p = max(beta.r, max(beta.g, beta.b));
            p = clamp(p, 0.05, 1.0);
            if (rand() > p)
                payload.alive = false;
            else 
                beta /= p;

            // --- Update payload ---
            payload.direction = Lworld;
            payload.origin = state.pos + EPS * Lworld;
            payload.pdf = new_bsdf_pdf;
            payload.cosTheta = state.cosTheta;
            payload.eta = state.eta;
            payload.N = state.N;
            payload.pos = state.pos;
            payload.bounce += 1;
            payload.throughput = beta;
        } else {
            payload.alive = false;
        }
    }

    // return;

    // --- Write back to payload queue if it's still alive ---
    // --- This step is very costly ---
#ifdef SHARED_WRITE // TODO:??
    uint tid = gl_LocalInvocationIndex;
    if (tid == 0) {
        localCount = 0;
    }
    memoryBarrierShared();
    
    barrier();
    uint localIndex = 0;
    if (payload.alive) {
        localIndex = atomicAdd(localCount, 1);
        localPayloads[localIndex] = payload;
        // localIndices[localIndex] = tid; // 可选，用于记录tid
    }
    barrier();

    if (tid == 0 && localCount > 0) {
        uint globalIndex = atomicAdd(s_QueueCounter, localCount);
        for (uint i = 0; i < localCount; ++i) {
            if (u_WriteIndex == 0) {
                s_Payload0[globalIndex + i] = localPayloads[i];
            } else {
                s_Payload1[globalIndex + i] = localPayloads[i];
            }
        }
    }
#else
    #ifdef INDIRECT_DISPATCH
        if (payload.alive) {
            uint queueTail = atomicAdd(s_QueueCounter, 1);
            u_WriteIndex == 0 ? s_Payload0[queueTail] = payload : s_Payload1[queueTail] = payload;
        }
    #else
        if (payload.alive) {
            u_WriteIndex == 0 ? s_Payload0[globalIdx] = payload : s_Payload1[globalIdx] = payload;
        }
    #endif
#endif

    // --- Accumulate ---
    vec3 accumulated = imageLoad(accumulatedImage, coords).rgb;
    accumulated += L;
    imageStore(accumulatedImage, coords, vec4(accumulated, 1.0));
}