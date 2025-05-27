#type compute
#version 460

#include "Common.glsl"
#include "../DeferredShading/BRDF.glsl"

// TODO: Use this binding mechanism for input uniforms
// layout(binding = 0) uniform sampler2D u_EquirectangularMap;

layout(binding = 0, rgba16f) uniform writeonly imageCube prefilterred;
layout(local_size_x = 16, local_size_y = 16) in;

uniform samplerCube u_CubeMap;
uniform float u_Roughness;
uniform float u_Size;
uniform int u_MipLevel;

vec3 PrefilterEnvMap(float Roughness, vec3 R) {
    vec3 N = R;
    vec3 V = R;
    vec3 prefilter = vec3(0);
    const uint NumSamples = 1024;
    float InvNumSamples = 1.0 / float(NumSamples);
    float TotalWeight = 0.0;
    float saTexel = 4.0 * PI / (6 * u_Size * u_Size);
    for (uint i = 0; i < NumSamples; i++) {
        vec2 Xi = SampleHammersley(i, InvNumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = reflect(-V, H);
        float LoN = dot(L, N);
        if (LoN > 0) {
            float NoH = max(dot(N, H), 0.0);
            float VoH = max(dot(V, H), 0.0);
            float D = D_GGX(Roughness, NoH);
            float pdf = D * NoH / (4.0 * VoH);
            pdf = max(pdf, 1e-5);
            float saSample = 1.0 / (float(NumSamples) * pdf);
            float mipLevel = Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            prefilter += textureLod(u_CubeMap, L, mipLevel).rgb * LoN;
            TotalWeight += LoN;
        }
    }
    return TotalWeight == 0.0 ? vec3(0) : prefilter / TotalWeight;
}

void main() {
    uint faceIdx = gl_GlobalInvocationID.z;
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / u_Size;  // normalized [0,1]
    uv = uv * 2.0 - 1.0; // [0, 1] -> [-1, 1]

    vec3 dir = GetSamplingVector(uv, faceIdx);
    vec3 Lo = PrefilterEnvMap(u_Roughness, dir);

    imageStore(prefilterred, ivec3(gl_GlobalInvocationID), vec4(Lo, 1.0));
}
