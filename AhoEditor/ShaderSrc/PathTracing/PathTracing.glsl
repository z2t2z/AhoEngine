#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "./PathTracingCommon.glsl"
#include "./GlobalVars.glsl"
#include "../UniformBufferObjects.glsl"
#include "./IntersectionTest.glsl"
#include "./rnd.glsl"

layout(binding = 0, rgba32f) uniform image2D outputImage;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

vec3 SampleInfiniteLight(Ray ray) {
    return vec3(0.0, 0.0, 0.7);
}

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

    float I = 1000.0;

    return I * (albedo / PI) * NdotL / attenuation;
}

vec3 LiRandomWalk(Ray ray) {
    vec3 L = vec3(0.0, 0.0, 0.0);
    vec3 beta = vec3(1.0, 1.0, 1.0);  // attenuation
    int depth = 0;
    vec3 eps = vec3(0.01, 0.01, 0.01);

    while (depth < LI_MAX_DEPTH) {
        HitInfo info;
        info.hit = false;
        info.t = -1.0;
        info.globalPrimtiveId = -1;
        info.meshId = -1;

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
            
        int meshId = p.meshId;

        vec3 hitPos = normalize(ray.origin + info.t * ray.direction);
        // hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;

        // return hitPos;

        L += beta * SampleEmissive();

        // BSDF bsdf = GetBSDF(ray, info);

        L += beta * SampleDirectLight(ray, hitPos, N, info, uv, meshId);

        break;

        float P_reflected = rand();
        // vec3 direction = P_reflected > 0.5 ? reflect()
        vec3 direction = vec3(rand(), rand(), rand());
        direction = normalize(direction);
        // uint64_t x;
        vec3 origin = ray.origin + info.t * direction;

        ray.direction = direction;
        ray.origin = origin;

        beta *= 0.5;
        depth += 1;
    }

    return L;
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    Ray ray = GetRayFromScreenSpace(
            vec2(pixelCoord), 
            vec2(imageSize(outputImage))
        );

    InitRNG(gl_GlobalInvocationID.xy, u_Frame);

    vec3 outColor = LiRandomWalk(ray);

    // float rnd = rand();

    // outColor = vec3(rnd, 0.0, 0.0);

    imageStore(outputImage, ivec2(gl_GlobalInvocationID.xy), vec4(outColor, 1.0));
}
