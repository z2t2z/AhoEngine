#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define SAMPLE_TEXTURE
#define OPT_SHADOW_TEST


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
#include "./Disney.glsl"

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
    vec3 L = vec3(0.0);
    vec3 beta = vec3(1.0);  // throughput, fr*cos/pdf
    int depth = 0;
    while (depth++ < 3) {
        HitInfo info = InitHitInfo();

        RayTrace(ray, info);

        if (!info.hit || info.t < 0 || info.globalPrimtiveId < 0) {
            L += beta * SampleInfiniteLight(ray);
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

        // float iblPdf = 0.0f;
        // vec3 iblL = SampleIBL(iblPdf, hitPos);

        // if (iblPdf != 0.0) {
        //     L += beta * iblL / iblPdf;
        // }

        float pdf = 0.0;
        vec3 wi;
        vec3 wo = -ray.direction;

        State state;
        state.baseColor = albedo;
        state.metallic = 0.1;
        state.roughness = 0.1;
        state.subsurface = 0.99;
        state.specTrans = 0.1;
        state.specular = 0.5;
        state.specularTint = 0.5;
        state.sheenTint = 0.5;
        state.ax = 0.01;
        state.ay = 0.01;
        state.ior = 1.3;
        
        // vec3 disneyDiffuse = DisneyDiffuse(state, wo, wi, N, pdf);
        vec3 disneySpec = DisneySpecular(state, wo, wi, N, pdf);

        if (pdf > 0.0) {
            beta *= 90.0 * disneySpec * abs(dot(N, wi)) / pdf;
        } else {
            continue;
        }

        ray.direction = normalize(wi);
        ray.origin = hitPos + EPS * wi;

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
