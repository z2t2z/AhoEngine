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
uniform int u_Frame;

#define MAX_PATHTRACE_DEPTH 8
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

        // Russian roulette
        {
            float p = max(beta.r, max(beta.g, beta.b));
            p = clamp(p, 0.05, 1.0);
            if (rand() > p) break;
            beta /= p;
        }
    }
    return L;
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
    InitRNG(gl_GlobalInvocationID.xy, u_Frame);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 siz = imageSize(accumulatedImage);
    if (pixelCoord.x >= siz.x || pixelCoord.y >= siz.y)
		return;
    Ray ray = GetRayFromScreenSpace(
                vec2(pixelCoord), 
                vec2(imageSize(accumulatedImage))
            );
    vec3 resColor = PathTrace(ray);
    vec4 accumulated = imageLoad(accumulatedImage, pixelCoord);
    accumulated += vec4(resColor, 1.0);
    imageStore(accumulatedImage, pixelCoord, accumulated);
}