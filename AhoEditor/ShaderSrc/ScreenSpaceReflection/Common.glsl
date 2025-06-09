#ifndef SSR_COMMON_GLSL
#define SSR_COMMON_GLSL

float DistanceSquared(vec2 lhs, vec2 rhs) {
    lhs -= rhs;
    return dot(lhs, lhs);
}
void swap(out float lhs, out float rhs) {
    float temp = lhs;
    lhs = rhs;
    rhs = temp;
}

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
vec3 SampleReflectionVector(vec3 rayDir, vec3 normal) {
    // TODO: GGXSampling to get reflected vector
    return normalize(reflect(rayDir, normal));
}
vec2 GetMipResolution(vec2 screen_size, int mip_level) {
    return screen_size * pow(0.5, mip_level);
}


#endif