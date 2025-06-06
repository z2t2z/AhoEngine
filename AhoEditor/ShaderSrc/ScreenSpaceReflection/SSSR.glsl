#type compute
#version 460

#include "Common.glsl"
#include "../Common/UniformBufferObjects.glsl"

layout(binding = 0, rgba16f) uniform writeonly image2D outputImage;

// G-Buffers
uniform sampler2D u_gPosition; 
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gPBR;
uniform sampler2D u_gDepth;

uniform float u_Near = 1000.0f;
uniform float u_Far = 0.1f;
uniform float u_MaxDisance = 100.0f; 
uniform int u_MipLevelTotal;

const int MAX_ITERATIONS = 100;
const float stepSiz = 0.04f;
const float thickNess = 0.1f;

vec4 RayMarchingHiZ(vec2 uv) {
    // View space
    vec3 beginPos = texture(u_gPosition, uv).xyz;
    beginPos = (u_View * vec4(beginPos, 1.0)).xyz;
    vec3 normal = texture(u_gNormal, uv).xyz;
    mat3 normalMatrix = transpose(inverse(mat3(u_View)));
    normal = normalMatrix * normal; // To view space
    normal = normalize(normal);

    vec3 viewDir = normalize(beginPos);
    vec3 rayDir = normalize(reflect(viewDir, normal));
    vec3 endPos = beginPos + rayDir * u_MaxDisance;

    // Clip space
    vec4 H0 = ViewSpaceToClipSpace(beginPos);
    vec4 H1 = ViewSpaceToClipSpace(endPos);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;

    // For interpolation
    vec3 Q0 = beginPos * k0;
    vec3 Q1 = endPos * k1;

    // NDC
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;

    // Screen space
    // ivec2 screenSize = textureSize(u_gDepth, 0);
    ivec2 screenSize = imageSize(outputImage);
    P0 = (P0 * 0.5f + 0.5f) * screenSize;
    P1 = (P1 * 0.5f + 0.5f) * screenSize;

    // To avoid line degeneration
    P1 += vec2((DistanceSquared(P0, P1) < 0.0001) ? 1.0 : 0.0);

    // Permute the direction for DDA
    vec2 delta = P1 - P0;
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) {
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2 dP = vec2(stepDir, delta.y * invdx);

    float stride = 1.0f;
    dP *= stride;
    dQ *= stride;
    dk *= stride;

    vec2 P = P0;
    vec3 Q = Q0;
    float k = k0;
    float endX = P1.x * stepDir;
    float prevZMaxEstimate = beginPos.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;

    int mipLevel = 0;
    int mipLevelCount = u_MipLevelTotal;

    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_ITERATIONS && P.x < endX; i++) {
        float d = exp2(mipLevel);
        P += dP * d;
        Q.z += dQ.z * d;
        k += dk * d;

        float rayDepth = Q.z / k;
        vec2 hitPixel = permute ? P.yx : P;
        vec2 bhitPixel = hitPixel;
        hitPixel /= pow(2, mipLevel);
        float sampleDepth = texelFetch(u_gDepth, ivec2(hitPixel), mipLevel).r;

        if (rayDepth < sampleDepth) {
            if (rayDepth + thickNess > sampleDepth) {
                if (mipLevel == 0 || true) {
                    result = texelFetch(u_gAlbedo, ivec2(bhitPixel), 0).rgb;
                    return vec4(result, 1.0f);
                    break;
                }
            }
            mipLevel = max(0, mipLevel - 1);
            P -= dP * d;
            Q.z -= dQ.z * d;
            k -= dk * d;
        }
        else {
            mipLevel = min(mipLevelCount - 1, mipLevel + 1);
        }
    }
    return vec4(0, 0, 1, 1);
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 siz = imageSize(outputImage);
	if (coord.x >= siz.x || coord.y >= siz.y)
		return;

    vec2 uv = vec2(coord) / vec2(siz);
    float d = texelFetch(u_gDepth, coord, 0).r;
    if (d == 1.0) {
        imageStore(outputImage, coord, vec4(1, 0, 0, 1));
        return;
    }

    vec4 L = RayMarchingHiZ(uv);
    imageStore(outputImage, coord, L);
}