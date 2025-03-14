#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "./PathTracingCommon.glsl"
#include "./GlobalVars.glsl"
#include "../UniformBufferObjects.glsl"
#include "./IntersectionTest.glsl"
#include "./Random.glsl"
#include "./LightSources/InfiniteLight.glsl"

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

vec3 SampleEmissive() {
    return vec3(0.0, 0.0, 0.0);
}


vec3 SampleDirectLight(Ray ray, vec3 hitPos, vec3 N, HitInfo info, vec2 uv, int textureHandleId) {
    vec3 albedo = vec3(0.9, 0.9, 0.9);
    if (textureHandleId >= 0) {
        TextureHandles handle = s_TextureHandles[textureHandleId];
        albedo = texture(handle.albedo, uv).rgb;
        // uint64_t albedoHandle = handle.albedo;
        // sampler2D albedoSampler = sampler2D(albedoHandle);
        // albedo = texture(albedoSampler, uv).rgb;
    }

    // return albedo;

    vec3 lightPos = vec3(u_LightPosition[0]);
    vec3 Ldir = normalize(lightPos - hitPos);

    float NdotL = max(0.0, dot(N, Ldir));
    float attenuation = length(lightPos - hitPos);
    attenuation *= attenuation;

    float I = 5000.0;

    return I * (albedo / PI) * NdotL / attenuation;
}

bool HasNormalMap(in out PrimitiveDesc p) {
    return (p.materialMask & NormalMapMask) > 0;
}

vec3 LiRandomWalk(Ray ray) {
    vec3 L = vec3(0.0, 0.0, 0.0);
    vec3 beta = vec3(1.0, 1.0, 1.0);  // attenuation
    int depth = 0;
    vec3 eps = vec3(0.01, 0.01, 0.01);

    while (depth < LI_MAX_DEPTH) {
        HitInfo info = InitHitInfo();

        RayTrace(ray, info);

        if (!info.hit) {
            L += beta * SampleInfiniteLight(ray);
            break;
        }

        if (info.t < 0) {
            break;
        }

        if (info.globalPrimtiveId < 0) {
            return vec3(0.0, 1.0, 0.0);
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
        if (HasNormalMap(p)) {
            TextureHandles handle = s_TextureHandles[meshId];
            vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
            T = normalize(T);
            T = normalize(T - dot(T, N) * N);
            vec3 B = cross(N, T); // Right-handed
            mat3 TBN = mat3(T, B, N);
            vec3 n = texture(handle.normal, uv).xyz; // Normal from normal map
            N = normalize(TBN * n);
        }        

        vec3 hitPos = normalize(ray.origin + info.t * ray.direction);

        L += beta * SampleEmissive();

        // BSDF bsdf = GetBSDF(ray, info);

        L += beta * SampleDirectLight(ray, hitPos, N, info, uv, meshId);

        // L = N;
        // break;

        // float P_reflected = rand();
        vec3 dir = reflect(ray.direction, -N);

        dir = vec3(rand(), rand(), rand());
        dir = normalize(dir);

        ray.direction = dir;
        ray.origin = hitPos;

        beta *= 0.2;
        depth += 1;
    }

    return L;
}

vec3 accumulate = vec3(0.0, 0.0, 0.0);

void main() {
    InitRNG(gl_GlobalInvocationID.xy, u_Frame);

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    Ray ray = GetRayFromScreenSpace(
            vec2(pixelCoord), 
            vec2(imageSize(accumulatedImage))
        );

    vec3 resColor = LiRandomWalk(ray);

    vec4 tmp = vec4(resColor, 1.0);
    // tmp = vec4(1.0, 1.0, 1.0, 1.0);

    vec4 accumulated = imageLoad(accumulatedImage, pixelCoord);

    accumulated += vec4(resColor, 1.0);

    imageStore(accumulatedImage, ivec2(gl_GlobalInvocationID.xy), accumulated);
}
