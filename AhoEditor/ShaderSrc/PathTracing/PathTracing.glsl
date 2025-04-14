#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define SAMPLE_TEXTURE
#define OPT_SHADOW_TEST

#include "PathTracingCommon.glsl"
#include "GlobalVars.glsl"
#include "LightSources/DirectLight.glsl"
#include "IntersectionTest.glsl"
#include "BxDFCommon.glsl"
#include "Math.glsl"
#include "Sampling.glsl"
#include "MicrofacetReflection.glsl"
#include "Disney.glsl"

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;
// layout(binding = 1, r32ui) uniform uimage2D tileMarkers;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

vec3 SampleEmissive() {
    return vec3(0.0, 0.0, 0.0);
}
bool HasNormalMap(in out PrimitiveDesc p) {
    return (p.materialMask & NormalMapMask) > 0;
}
bool HasAlbedoMap(in out PrimitiveDesc p) {
    return (p.materialMask & AlbedoMapMask) > 0;
}
vec3 GetAlbedo(vec2 uv, int textureHandleId) {
    TextureHandles handle = s_TextureHandles[textureHandleId];
    if (int64_t(handle.albedo) == 0) {
        return vec3(1.0, 1.0, 1.0);
    }
    ivec2 tsize = textureSize(handle.albedo, 0);
    tsize.x = min(int(uv.x * float(tsize.x)), tsize.x - 1);
    tsize.y = min(int(uv.y * float(tsize.y)), tsize.y - 1);
    return texture(handle.albedo, uv).rgb;
}

void RetrieveMaterial(out State state, out Material mat, int meshId, vec2 uv) {
    // TextureHandles handle = s_TextureHandles[meshId];
    // if (int64_t(handle.normal) != 0) {
    //     vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
    //     // float handedness = p.v[0].tangent.w;
    //     T = normalize(T);
    //     T = normalize(T - dot(T, N) * N);
    //     vec3 B = cross(N, T); // Right-handed
    //     mat3 TBN = mat3(T, B, N);
    //     vec3 n = texture(handle.normal, uv).xyz * 2.0 - 1.0; // Normal from normal map
    //     N = normalize(TBN * n);

    //     state.N = N;
    // }
    // if (int64_t(handle.albedo) != 0) {
    //     mat.baseColor = texture(handle.albedo, uv).rgb;
    // }
}

#define MAX_TRACING_DEPTH 5
vec3 PathTrace(Ray ray) {
    vec3 L = vec3(0.0);
    vec3 beta = vec3(1.0);  // throughput
    State state = InitState();
    for (int depth = 0; depth < MAX_TRACING_DEPTH; ++depth) {
        HitInfo info = InitHitInfo();
        RayTrace(ray, info);
        if (!info.hit) {
            vec4 env = SampleInfiniteLight(ray);
            float misWeight = depth > 0 ? PowerHeuristicPdf(state.pdf, env.w) : 1.0; // Need better understanding of this
            L += misWeight * beta * env.rgb;
            break;
        }

        PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
        float u = info.uv.x;
        float v = info.uv.y;
        float w = 1.0 - u - v;
        vec3 N = w * p.v[0].normal + u * p.v[1].normal + v * p.v[2].normal;
        N = normalize(N);

        vec2 uv = vec2(w * p.v[0].u + u * p.v[1].u + v * p.v[2].u,
            w * p.v[0].v + u * p.v[1].v + v * p.v[2].v); 

        // Retrieve normal from normal map if there is
        int meshId = p.meshId;

#ifdef SAMPLE_TEXTURE
        TextureHandles handle = s_TextureHandles[meshId];
        if (int64_t(handle.normal) != 0) {
            vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
            // float handedness = p.v[0].tangent.w;
            T = normalize(T);
            T = normalize(T - dot(T, N) * N);
            vec3 B = cross(N, T); // Right-handed
            mat3 TBN = mat3(T, B, N);
            ivec2 tsize = textureSize(handle.normal, 0);
            tsize.x = int(uv.x * float(tsize.x));
            tsize.y = int(uv.y * float(tsize.y));
            vec3 n = texture(handle.normal, uv).xyz * 2.0 - 1.0;
            N = normalize(TBN * n);
        }
        state.baseColor = GetAlbedo(uv, meshId);
#endif

        N = dot(N, ray.direction) < 0.0 ? N : -N; // flip normal if inside the object
        state.N = N;
        
        vec3 hitPos = ray.origin + info.t * ray.direction; 
        state.pos = hitPos;     

        // L += beta * SampleEmissive();
        L += beta * SampleDirectLight(state, ray);

        float pdf = 0.0;
        vec3 wi;
        vec3 wo = -ray.direction;
        // vec3 f = DisneyDiffuse(state, wo, wi, N, pdf);
        vec3 f = DisneySpecular(state, wo, wi, N, pdf);
        // vec3 f = DisneyClearcoat(state, wo, wi, N, pdf);
        // vec3 f = DisneyGlass(state, wo, wi, N, pdf);

        if (pdf > 0.0) {
            beta *= f / pdf;
            state.pdf = pdf;
        } else {
            continue;
        }

        ray.direction = normalize(wi);
        ray.origin = hitPos + EPS * wi;
        state.cosTheta = dot(ray.direction, N);
        state.eta = dot(ray.direction, N) < 0.0 ? (1.0 / 1.5) : 1.5;
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
