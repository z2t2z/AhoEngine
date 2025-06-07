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
    return uv.x > 0.0f && uv.y > 0.0f && uv.x < 1.0f && uv.y < 1.0f;
}




#endif