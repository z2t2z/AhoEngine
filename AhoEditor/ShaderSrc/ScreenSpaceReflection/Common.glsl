#ifndef SSR_COMMON_GLSL
#define SSR_COMMON_GLSL

#include "../Common/UniformBufferObjects.glsl"

vec4 ViewSpaceToClipSpace(vec3 pos) {
    vec4 p = vec4(pos, 1.0f);
    vec4 res = u_Projection * p;
    return res;
}
float DistanceSquared(vec2 lhs, vec2 rhs) {
    lhs -= rhs;
    return dot(lhs, lhs);
}
void swap(out float lhs, out float rhs) {
    float temp = lhs;
    lhs = rhs;
    rhs = temp;
}

// Reference: https://www.jpgrenier.org/ssr.html
// void RayMarchHiZ(vec3 startPos, vec3 rayDir, out vec2 hitUV, int MAX_ITERATIONS = 128) {
//     int iters = 0;
//     int currMipLevel = 0;
//     while (iters < MAX_ITERATIONS) {

//         iters += 1;
//     }
// }


#endif