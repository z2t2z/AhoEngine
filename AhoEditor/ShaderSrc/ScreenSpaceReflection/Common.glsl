#ifndef SSR_COMMON_GLSL
#define SSR_COMMON_GLSL

#include "Sampling.glsl"

// Errors
vec3 ReconstructViewPosition(vec2 uv, float depth, mat4 invProj) {
    vec4 ndc;
    ndc.xy = uv * 2.0 - 1.0;
    ndc.z = depth * 2.0 - 1.0;
    ndc.w = 1.0;
    vec4 viewPos = invProj * ndc;
    return viewPos.xyz / viewPos.w;
}
vec4 ViewToNDC(vec3 viewPos, mat4 proj) {
    vec4 tmp = proj * vec4(viewPos, 1);
    return tmp / tmp.w;
}
// view space to NDC
vec3 ToNDC(vec3 pos, mat4 proj) {
    vec4 tmp = proj * vec4(pos, 1);
    return tmp.xyz / tmp.w;
}
bool ValidUV(vec2 uv) {
    return uv.x >= 0.0f && uv.y >= 0.0f && uv.x <= 1.0f && uv.y <= 1.0f;
}

vec3 ScreenSpaceToViewSpace(vec3 pos, mat4 invCamProj) {
    pos = pos * 2.0 - 1.0;
    vec4 tmp = invCamProj * vec4(pos, 1.0);
    return tmp.xyz / tmp.w;
}
vec3 ScreenSpaceToWorldSpace(vec3 pos, mat4 mat) {
    pos = pos * 2 - 1;
    vec4 tmp = mat * vec4(pos, 1);
    return tmp.xyz / tmp.w;
}
vec3 ProjectVsDirToSsDir(vec3 vs_pos, vec3 vs_dir, vec3 ss_origin, mat4 camProj) {
    vec3 end = vs_pos + vs_dir;
    vec4 pj = camProj * vec4(end, 1.0);
    pj.xyz /= pj.w;
    pj = pj * 0.5 + 0.5;
    vec3 offsetted = pj.xyz;
    return offsetted - ss_origin;
}

// Simple random, could use noise texture
float Random1D(vec2 co) {
    return fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453);
}
vec2 SampleRandom2D(vec2 uv) {
    float rnd1 = Random1D(uv);
    float rnd2 = Random1D(uv.yx);
    return vec2(rnd1, rnd2);
}

// #define PERFECT_SSR_REFLECTION
vec3 SampleReflectionVector(vec3 rayDir, vec3 normal, float roughness, vec2 uv) {
#ifdef PERFECT_SSR_REFLECTION
    return normalize(reflect(-rayDir, normal));
#endif
    mat3 tbn = ConstructTBN(normal);
    vec3 V = normalize(WorldToLocal(rayDir, tbn));
    float alpha_x = roughness * roughness;
    float alpha_y = alpha_x;
    vec2 rnd = SampleRandom2D(uv);
    vec3 H = normalize(SampleGGXVNDF(V, alpha_x, alpha_y, rnd.x, rnd.y)); // V and H are in the same hemisphere
    vec3 L = normalize(reflect(-V, H));
    return LocalToWorld(L, tbn);
}
vec2 GetMipResolution(vec2 screen_size, int mip_level) {
    return screen_size * pow(0.5, mip_level);
}


#endif