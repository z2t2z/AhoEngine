#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define PATH_TRACING
#define OPT_SHADOW_TEST
#define OPT_INFINITE_LIGHT

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
    const TextureHandles handle = s_TextureHandles[p.meshId];
    Material mat;// = InitMaterial();
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

#define MAX_PATHTRACE_DEPTH 5
vec3 PathTrace(Ray ray) {
    vec3 L = vec3(0.0);
    vec3 beta = vec3(1.0);  // throughput
    float pdf = 1.0;        // pdf of the last sample
    State state = InitState();
    for (int depth = 0; depth < MAX_PATHTRACE_DEPTH; ++depth) {
        HitInfo info = InitHitInfo();
        RayTrace(ray, info);

        // Evaluete infinite light for escaped ray
        if (!info.hit) {
            vec4 Le_pdf = SampleInfiniteLight(ray);
            vec3 Le = Le_pdf.rgb;
            if (Le_pdf.a == -1 || depth == 0) {
                L += beta * Le;
            } else {
                float misWeight = PowerHeuristicPdf(pdf, Le_pdf.a); // Need better understanding of this
                L += misWeight * beta * Le;
            }
            break;
        }

        PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
        RetrievePrimInfo(state, p, info.uv);

        // Evaluate emission from surface hit by ray
        if (state.material.emissiveScale > 0.0) {
            vec3 Le = state.material.emissive * state.material.emissiveScale;
            if (depth == 0) {
                L += beta * Le;
            } else {
                // float tpdf = 1.0; 
                // float pdf2 = pdf * pdf;
                // float tpdf2 = tpdf * tpdf;
                // float denom = (tpdf2 + pdf2);
                // L += (tpdf / denom) * beta * Le / tpdf + (pdf / denom) * Le / pdf;
            }
        }
        
        L += beta * SampleDirectLight(state, ray); 

        state.cosTheta = dot(state.N, -ray.direction); // cosTheta changed during ibl sampling, so set it again

        vec3 Lworld;
        float tpdf;
        vec3 f = Sample(state, -ray.direction, Lworld, tpdf);
        if (tpdf > 0.0) {
            pdf = tpdf;
            beta *= f / pdf;
        } else {
            break;
        }
        ray.direction = Lworld;
        ray.origin = state.pos + EPS * Lworld;

        // TODO: Russian roulette

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
    vec3 resColor = PathTrace(ray);
    vec4 accumulated = imageLoad(accumulatedImage, pixelCoord);
    accumulated += vec4(resColor, 1.0);
    imageStore(accumulatedImage, pixelCoord, accumulated);
}
