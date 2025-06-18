#type compute
#version 460 core

#include "Common.glsl"
#include "../DeferredShading/BRDF.glsl"

layout(binding = 0, rgba16f) restrict writeonly uniform imageCube outputTexture;
layout(local_size_x = 16, local_size_y = 16) in;

vec2 BRDFIntegrate(float NoV, float Roughness) {
    NoV = max(NoV, Epsilon);
    vec3 V;
    V.x = sqrt(1.0 - NoV * NoV);
    V.y = 0.0;
    V.z = NoV;
    float A = 0.0;
    float B = 0.0; 
    vec3 N = vec3(0.0, 0.0, 1.0);
    const uint NumSamples = 1024u;
    float InvNumSamples = 1.0 / float(NumSamples);
    for (uint i = 0u; i < NumSamples; i++) {
        vec2 Xi = SampleHammersley(i, InvNumSamples);
        vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float LoN = dot(L, N);
        float NoV = max(dot(N, V), 0.0);
        float NoL = max(dot(N, L), 0.0);
        float NoH = max(dot(N, H), 0.0);
        float VoH = max(dot(V, H), 0.0);

        if (LoN > 0.0) {
            float G = G_Smith(NoV, NoL, Roughness);
            float G_Vis = (G * VoH) / (NoH * NoV);
            float Fc = pow(1.0 - VoH, 5.0); // FresnelSchlick
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return vec2(A, B) * InvNumSamples;
}

void main() {
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(outputTexture));
    vec2 brdf = BRDFIntegrate(uv.x, uv.y);
    imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(brdf, 0.0, 0.0));
}