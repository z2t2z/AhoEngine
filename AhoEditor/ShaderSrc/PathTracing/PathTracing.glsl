#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

// #define SAMPLE_TEXTURE
#define OPT_SHADOW_TEST

#include "PathTracingCommon.glsl"
#include "GlobalVars.glsl"
#include "LightSources/DirectLight.glsl"
#include "IntersectionTest.glsl"
#include "BxDFCommon.glsl"
#include "Math.glsl"
#include "Sampling.glsl"
#include "MicrofacetReflection.glsl"
#include "DisneyPrincipled.glsl"

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

void RetrievePrimInfo(out State state, in PrimitiveDesc p, vec2 uv) {
    Material mat;// = InitMaterial();
    float u = uv.x, v = uv.y, w = 1.0 - u - v;
    
    vec3 N = w * p.v[0].normal + u * p.v[1].normal + v * p.v[2].normal;
    N = normalize(N);

    vec3 hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;  // Don't use ray.origin + info.t * ray.direction
    state.pos = hitPos;
    
    vec2 TextureUV = vec2(w * p.v[0].u + u * p.v[1].u + v * p.v[2].u,
            w * p.v[0].v + u * p.v[1].v + v * p.v[2].v); 
    
    // if (p.meshId == 3) {
    //     mat.baseColor = X;
    //     return;
    // }
    const TextureHandles handle = s_TextureHandles[p.meshId];
    if (int64_t(handle.albedoHandle) != 0) {
        mat.baseColor = texture(handle.albedoHandle, TextureUV).rgb;
    } else {
        mat.baseColor = handle.baseColor;
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
    N = normalize(N);
    state.N = N;    
    
    if (int64_t(handle.roughnessHandle) != 0) {
        mat.roughness = texture(handle.roughnessHandle, TextureUV).r;
    } else {
        mat.roughness = handle.roughness;
    }
    
    if (int64_t(handle.metallicHandle) != 0) {
        mat.metallic = texture(handle.metallicHandle, TextureUV).r;
    } else {
        mat.metallic = handle.metallic;
    }

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
    mat.ax = handle.ax;
    mat.ay = handle.ay;
    
    // CalDistParams(mat.anisotropic, mat.roughness, mat.ax, mat.ay); // Done in cpu side

    state.material = mat;
}

#define MAX_PATHTRACE_DEPTH 6
vec3 PathTrace(Ray ray) {
    vec3 L = vec3(0.0);
    vec3 beta = vec3(1.0);  // throughput
    State state = InitState();
    for (int depth = 0; depth < MAX_PATHTRACE_DEPTH; ++depth) {
        HitInfo info = InitHitInfo();
        RayTrace(ray, info);
        if (!info.hit) {
            vec4 env = SampleInfiniteLight(ray);
            float misWeight = depth > 0 ? PowerHeuristicPdf(state.pdf, env.w) : 1.0; // Need better understanding of this
            L += misWeight * beta * env.rgb;
            break;
        }

        PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
        RetrievePrimInfo(state, p, info.uv);
        {
            // L = state.material.baseColor;
            // break;
        }
        state.cosTheta = dot(state.N, -ray.direction);
        
        L += beta * SampleDirectLight(state, ray); 

        float pdf = 0.0;
        vec3 wi;
        vec3 wo = -ray.direction;
        // seperate component test
        {
            // DotProducts dp;
            // mat3 tbn = ConstructTBN(state.N);
            // vec3 V = WorldToLocal(-ray.direction, tbn); 
            // vec3 L = SampleCosineHemisphere();
            // vec3 H = normalize(L + V);
            // SetDotProducts(L, V, H, Y, dp);
            // float pdf = 0.0;
            // vec3 f = eval_diffuse(state, dp, V, L, H, pdf);
            // vec3 wi = LocalToWorld(L, tbn);
            // if (pdf > 0.0) {
            //     beta *= f * abs(dp.LdotN) / pdf;
            // } else {
            //     break;
            // }

            // ray.direction = normalize(wi);
            // ray.origin = state.pos + EPS * wi;
            // continue;
        }

        {
            // vec3 Sample(inout State state, vec3 Vworld, out vec3 Lworld, out float pdf);
            vec3 Lworld;
            float pdf;
            vec3 f = Sample(state, -ray.direction, Lworld, pdf);
            if (pdf > 0.0) {
                beta *= f / pdf;
            } else {
                break;
            }
            ray.direction = Lworld;
            ray.origin = state.pos + EPS * Lworld;
            continue;
        }

    }

    return L;
}

void main() {
    InitRNG(gl_GlobalInvocationID.xy, u_Frame);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    // if (u_Frame == 1) {
    //     ivec2 tileCoord = ivec2(gl_WorkGroupID.xy);
    //     ivec2 tileOrigin = tileCoord * u_TileSize;
    //     uint prev = imageAtomicExchange(tileMarkers, tileCoord, 1);
    //     if (prev == 0) {
    //         ivec2 samplePos = tileCoord * u_TileSize + u_TileSize / 2;
    //         Ray ray = GetRayFromScreenSpace(
    //                                 samplePos, 
    //                                 vec2(imageSize(accumulatedImage)));
    //         vec3 color = PathTrace(ray);
    //         imageStore(accumulatedImage, tileCoord, vec4(color, 1.0));
    //     }
    //     memoryBarrier();

    //     vec4 tileColor = imageLoad(accumulatedImage, tileCoord);
    //     imageStore(accumulatedImage, pixelCoord, tileColor);
    //     return;
    // }
    // float SKIP_P = u_Frame == 1 ? 0.1 : 1.0;
    // if (u_Frame == 1) {
    //     float skip = rand();
    //     if (skip > SKIP_P) {
    //         return;
    //     }
    // }

    Ray ray = GetRayFromScreenSpace(
                vec2(pixelCoord), 
                vec2(imageSize(accumulatedImage))
            );
    vec3 resColor = PathTrace(ray);
    vec4 accumulated = imageLoad(accumulatedImage, pixelCoord);
    accumulated += vec4(resColor, 1.0);
    imageStore(accumulatedImage, pixelCoord, accumulated);
}
