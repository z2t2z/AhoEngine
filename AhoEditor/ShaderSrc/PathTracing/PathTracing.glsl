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
#include "./Math.glsl"
#include "./Sampling.glsl"
#include "./BxDFCommon.glsl"
#include "./MicrofacetReflection.glsl"

#define SAMPLE_TEXTURE
#define OPT_SHADOW_TEST

layout(binding = 0, rgba32f) uniform image2D accumulatedImage;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

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
        return vec3(1.0, 0.0, 0.0);
    }
    ivec2 tsize = textureSize(handle.albedo, 0);
    tsize.x = min(int(uv.x * float(tsize.x)), tsize.x - 1);
    tsize.y = min(int(uv.y * float(tsize.y)), tsize.y - 1);
    return texture(handle.albedo, uv).rgb;
    // return texelFetch(handle.albedo, tsize, 0).rgb;
}

bool VisibilityTest(vec3 from, vec3 to) {
#ifndef OPT_SHADOW_TEST
    return true;
#endif
    Ray ray;
    ray.direction = normalize(to - from);
    ray.origin = from + 0.01 * ray.direction;
    if (AnyHit(ray, length(to - from))) {
        return false;
    }
    return true;
}

vec3 SampleDirectLight(Ray ray, vec3 hitPos, vec3 N, HitInfo info, vec2 uv, int textureHandleId) {
    vec3 lightPos = vec3(u_LightPosition[0]);
    vec3 Ldir = normalize(lightPos - hitPos);

    if (VisibilityTest(hitPos, lightPos)) {
        float NdotL = max(0.0, dot(N, Ldir));
        float attenuation = length(lightPos - hitPos);
        attenuation *= attenuation;
        float I = 1000.0;
        return I * InvPI * NdotL / attenuation * vec3(1.0, 1.0, 1.0);
    }

    return vec3(0.0, 0.0, 0.0);
}

vec3 LiRandomWalk(Ray ray) {
    vec3 L = vec3(0.0, 0.0, 0.0);
    vec3 beta = vec3(1.0, 1.0, 1.0);  // also called throughput, fr*cos/pdf
    int depth = 0;

    while (depth < 3) {
        HitInfo info = InitHitInfo();

        RayTrace(ray, info);

        if (!info.hit || info.t < 0 || info.globalPrimtiveId < 0) {
            L += beta * 1.0 * SampleInfiniteLight(ray);
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
        vec3 albedo = vec3(0.9, 0.9, 0.9);

#ifdef SAMPLE_TEXTURE
        if (HasNormalMap(p)) {
            TextureHandles handle = s_TextureHandles[meshId];
            if (int64_t(handle.normal) != 0) {
                vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
                T = normalize(T);
                T = normalize(T - dot(T, N) * N);
                vec3 B = cross(N, T); // Right-handed
                mat3 TBN = mat3(T, B, N);

                ivec2 tsize = textureSize(handle.normal, 0);
                tsize.x = int(uv.x * float(tsize.x));
                tsize.y = int(uv.y * float(tsize.y));
                vec3 n = texture(handle.normal, uv).xyz; // Normal from normal map
                // vec3 n = texelFetch(handle.normal, tsize, 0).xyz;
                N = normalize(TBN * n);
            }
        }        
        if (HasAlbedoMap(p)) {
            albedo = GetAlbedo(uv, meshId);
        }
#endif

        vec3 hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position; // <==> ray.origin + info.t * ray.direction; 

        L += beta * SampleEmissive();
        L += beta * albedo * SampleDirectLight(ray, hitPos, N, info, uv, meshId);

        float pdf;
        vec3 wi;

        vec3 wo = -ray.direction;
        mat3 tbn = CalTBN(N);
        wo = WorldToLocal(wo, tbn);
        vec3 fv = Sample_f_MR(wo, wi, pdf);

        // vec3 H = SampleGGXVNDF(wo, 0.01, 0.01, rand(), rand());
        // if (H.z < 0) {
        //     H = -H;
        // }
        // wi = normalize(reflect(-wo, H));

        if (wi.x == 0 && wi.y == 0 && wi.z == 0) {
            L = vec3(1.0, 0.0, 0.0);
            break;
        }

        wi = LocalToWorld(wi, tbn);

        // vec3 uniwi = SampleCosineHemisphere();
        // float unipdf = CosineHemispherePDF(uniwi);
        // uniwi = normalize(uniwi);
        
        if (pdf == 0) {
            L = vec3(1.0, 0.0, 0.0);
            break;
        }
        if (isnan(wi.x) || isnan(wi.y) || isnan(wi.z)) {
            L = vec3(1.0, 0.0, 0.0);
            break;
        }

        wi = normalize(wi);
        // beta *= InvPI * albedo * abs(uniwi.y) / unipdf;
        beta *= fv * albedo * abs(dot(N, wi)) / pdf;

        // uniwi = LocalToWorld(uniwi, N);        
        vec3 dir = wi;
        // dir = uniwi;

        ray.direction = dir;
        ray.origin = hitPos + 0.0001 * dir;

        depth += 1;
    }

    return L;
}

void main() {
    InitRNG(gl_GlobalInvocationID.xy, u_Frame);

    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    Ray ray = GetRayFromScreenSpace(
                vec2(pixelCoord), 
                vec2(imageSize(accumulatedImage))
            );

    vec3 resColor = LiRandomWalk(ray);

    vec4 accumulated = imageLoad(accumulatedImage, pixelCoord);

    accumulated += vec4(resColor, 1.0);
    
    imageStore(accumulatedImage, ivec2(gl_GlobalInvocationID.xy), accumulated);
}
