#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define PATH_TRACING
#define OPT_SHADOW_TEST
#define OPT_INFINITE_LIGHT

#include "../PathTracingCommon.glsl"
#include "../GlobalVars.glsl"
#include "../LightSources/DirectLight.glsl"
#include "../IntersectionTest.glsl"
#include "../BxDFCommon.glsl"
#include "../Math.glsl"
#include "../Sampling.glsl"
#include "../MicrofacetReflection.glsl"
#include "../DisneyPrincipled.glsl"

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;

uniform int u_Frame = 1;
uniform int u_MaxBounce = 8;
uniform int u_WriteIndex;
uniform int u_ReadIndex;
uniform int u_SrcWidth;
uniform int u_SrcHeight;

void RetrievePrimInfo(out State state, in PrimitiveDesc p, vec2 uv);

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() {
    // --- Consume payload from queue ---
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= u_SrcWidth || coords.y >= u_SrcHeight)
		return;

	int pixelIndex = coords.y * u_SrcWidth + coords.x;
    Payload payload;
	if (u_ReadIndex == 0) {
		payload = s_Payload0[pixelIndex];
        s_Payload0[pixelIndex].alive = false;
    }
    else {
		payload = s_Payload1[pixelIndex];
        s_Payload1[pixelIndex].alive = false;
    }

    if (!payload.alive || payload.bounce > u_MaxBounce)
        return;

    InitRNG(vec2(coords), u_Frame);

    // --- Intersect scene ---
    Ray ray;
    ray.origin = payload.origin;
    ray.direction = payload.direction;
    HitInfo info = InitHitInfo();
    RayTrace(ray, info);

    // --- State from last bounce ---
    vec3 beta = payload.throughput;
    vec3 L = vec3(0);
    float pdf = payload.pdf;
    
    State state;
    state.cosTheta = payload.cosTheta;
    state.eta = payload.eta;
    state.N = payload.N;
    state.pos = payload.pos;

    // --- Miss ray: Evaluate environment light ---
    if (!info.hit) {
        vec4 Le_pdf = SampleInfiniteLight(ray);
        vec3 Le = Le_pdf.rgb;
        if (Le_pdf.a == -1 || payload.bounce == 1) {
            L += beta * Le;
        } else {
            float misWeight = PowerHeuristicPdf(pdf, Le_pdf.a);
            L += misWeight * beta * Le;
        }
        payload.alive = false;
    } 
    // --- Hit ray: bsdf sample/evaluation, and write to payload queue ---
    else {
        PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
        RetrievePrimInfo(state, p, info.uv); // Get material info
        
        L += beta * SampleDirectLight(state, ray); 

        state.cosTheta = dot(state.N, -ray.direction); // cosTheta changed during IBL sampling, so set it again

        // --- Sample next ray --- //TODO: RR
        vec3 Lworld;
        float new_bsdf_pdf;
        vec3 f = Sample(state, -ray.direction, Lworld, new_bsdf_pdf);
        if (new_bsdf_pdf > 0.0 && payload.bounce < u_MaxBounce && length(beta) >= 1e-3) {
            // --- Update payload ---
            beta *= f / new_bsdf_pdf;
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

    if (u_WriteIndex == 0)
        s_Payload0[pixelIndex] = payload;
    else
        s_Payload1[pixelIndex] = payload;

    // --- Accumulate ---
    vec3 accumulated = imageLoad(accumulatedImage, coords).rgb;
    accumulated += L;
    imageStore(accumulatedImage, coords, vec4(accumulated, 1.0));
}

void RetrievePrimInfo(out State state, in PrimitiveDesc p, vec2 uv) {
    const TextureHandles handle = s_TextureHandles[p.meshId];
    Material mat;
    mat.baseColor = handle.baseColor;
    mat.metallic = handle.metallic;
    mat.emissive = handle.emissive;
    mat.emissiveScale = handle.emissiveScale;
    mat.subsurface = handle.subsurface;
    mat.specular = handle.specular;
    mat.specTint = handle.specTint;
    mat.specTrans = handle.specTrans;
    mat.ior = handle.ior;
    mat.clearcoat = handle.clearcoat;
    mat.clearcoatGloss = handle.clearcoatGloss;
    mat.anisotropic = handle.anisotropic;
    mat.sheenTint = handle.sheenTint;
    mat.sheen = handle.sheen;
    mat.roughness = handle.roughness;
    mat.ax = handle.ax;
    mat.ay = handle.ay;

    float u = uv.x, v = uv.y, w = 1.0 - u - v;
    vec3 N = normalize(w * p.v[0].normal + u * p.v[1].normal + v * p.v[2].normal);
    vec3 hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;  // Don't use ray.origin + info.t * ray.direction
    state.pos = hitPos;
    vec2 TextureUV = vec2(w * p.v[0].u + u * p.v[1].u + v * p.v[2].u, w * p.v[0].v + u * p.v[1].v + v * p.v[2].v); 
    if (int64_t(handle.albedoHandle) != 0) {
        mat.baseColor = texture(handle.albedoHandle, TextureUV).rgb;
    }
    mat.baseColor = pow(mat.baseColor, vec3(2.2));
    if (int64_t(handle.normalHandle) != 0) {
        vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
        T = normalize(T);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T); // Right-handed
        mat3 TBN = mat3(T, B, N);
        vec3 n = texture(handle.normalHandle, TextureUV).xyz * 2.0 - 1.0;
        N = normalize(TBN * n);
    } 
    state.N = N;    
    if (int64_t(handle.roughnessHandle) != 0) {
        mat.roughness = texture(handle.roughnessHandle, TextureUV).r;
        CalDistParams(mat.anisotropic, mat.roughness, mat.ax, mat.ay); 
    }
    if (int64_t(handle.metallicHandle) != 0) {
        mat.metallic = texture(handle.metallicHandle, TextureUV).r;
    }
    state.material = mat;
}