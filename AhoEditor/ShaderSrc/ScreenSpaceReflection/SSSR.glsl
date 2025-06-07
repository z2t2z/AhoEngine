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

uniform float u_MaxDisance = 128.0f; 
uniform int u_MipLevelTotal;

const int MAX_ITERATIONS = 100;
const float stepSiz = 0.04f;
const float thickNess = 0.01f;


// start: (current_uv, invZ(ndc), ?)
// dir: 
vec4 AdvanceRay(vec4 start, vec4 dir, int currMipLevel) {
    vec2 crossStep = vec2(dir.x >= 0 ? 1 : -1, dir.y >= 0 ? 1 : -1);
    vec2 crossOffset = crossStep * 0.00001;
    
    ivec2 extent = textureSize(u_gDepth, currMipLevel);
    vec2 invExtent = 1.0 / extent;
    ivec2 cellIdx = ivec2(floor(start.xy) * extent);
    vec2 planes = vec2(cellIdx) / extent + cellIdx * crossStep; // Try ray march one step in uv space

    vec2 solutions = (planes - start.xy) / dir.xy;
    vec4 isect = start + dir * min(solutions.x, solutions.y);
    isect.xy += (solutions.x < solutions.y) ? vec2(crossOffset.x, 0) : vec2(0, crossOffset.y);

    return isect;
}


// beginPos, rayDir must be in view space
// Returns true if there is a valid hit
bool RayMarchingHiZ(vec3 beginPos, vec3 rayDir, mat4 camProj, ivec2 screenSize, float maxDistance, float stride, float thickness, int mipLevelCount, vec2 hitUV, out vec4 debugL) {
    vec3 endPos = beginPos + rayDir * maxDistance;
    vec4 H0 = camProj * vec4(beginPos, 1.0);
    vec4 H1 = camProj * vec4(endPos, 1.0);
    float invW0 = 1.0 / H0.w;
    float invW1 = 1.0 / H1.w;

    // NDC
    vec2 P0 = H0.xy * invW0;
    vec2 P1 = H1.xy * invW1;

    // For interpolation
    vec3 Q0 = beginPos * invW0;
    vec3 Q1 = endPos * invW1;

    // Screen space
    P0 = (P0 * 0.5f + 0.5f) * screenSize; // texel coord in float
    P1 = (P1 * 0.5f + 0.5f) * screenSize;

    // Avoid line degeneration
    P1 += vec2((DistanceSquared(P0, P1) < 0.0001) ? 1.0 : 0.0);

    // Permute the direction for DDA, make dir.x is the main direction
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
    float dk = (invW1 - invW0) * invdx;
    vec2 dP = vec2(stepDir, delta.y * invdx);

    dP *= stride;
    dQ *= stride;
    dk *= stride;

    vec2 P = P0;
    vec3 Q = Q0;
    float k = invW0;
    float endX = P1.x * stepDir;

    int mipLevel = 0;
    for (int i = 0; i < MAX_ITERATIONS && P.x < endX && ValidUV(P); i++) {
        float d = exp2(mipLevel);
        P += dP * d;        // trace screen coords
        Q.z += dQ.z * d;    // trace view space pos
        k += dk * d;        // trace inv w

        float rayDepth = Q.z / k;
        vec2 hitPixel = permute ? P.yx : P;
        ivec2 tryhitUV = ivec2(hitPixel);
        hitPixel /= pow(2, mipLevel);
        float sampleDepth = texelFetch(u_gDepth, ivec2(hitPixel), mipLevel).r;

        if (rayDepth < sampleDepth) {
            if (rayDepth + thickness > sampleDepth) {
                if (mipLevel == 0 || true) {
                    hitUV = tryhitUV / vec2(screenSize);
                    debugL = vec4(0, 1, 0, 1);
                    return true;
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
    vec2 rUV = permute ? P.yx : P.xy;
    rUV /= screenSize;
    debugL = vec4(1, 0, 0, 1);
    if (ValidUV(rUV)) {
        debugL = vec4(1, 1, 1, 1);
    }
    return false;
}

// Low accuracy, only for testing
bool NaiveRayMarching(vec3 beginPos, vec3 rayDir, out vec2 hitUV, int maxIterations = 256, float deltaStep = 0.04, float thickNess = 0.04) { 
    vec3 reflectDir = rayDir;
    for (int i = 0; i < maxIterations; i++) {  
        vec3 nxtPos = beginPos + reflectDir * deltaStep * i; 
        vec3 ndc = ToNDC(nxtPos, u_Projection);
        vec2 uv = ndc.xy * 0.5f + 0.5f;
        if (!ValidUV(uv)) 
            return false;
        float sampleDepth = textureLod(u_gPosition, uv, 0).z;
        if (i > 0 && sampleDepth > nxtPos.z && sampleDepth < nxtPos.z + thickNess) {
            hitUV = uv;
            return true;
        } 
    }
    return false;
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 siz = imageSize(outputImage);
	if (coord.x >= siz.x || coord.y >= siz.y)
		return;

    vec2 uv = (vec2(coord) + vec2(0.5)) / vec2(siz);

    float d = texelFetch(u_gDepth, coord, 0).r;
    if (d == 1.0) {
        imageStore(outputImage, coord, vec4(1, 0, 0, 1));
        return;
    }

    vec3 beginPos = texelFetch(u_gPosition, coord, 0).xyz; // in view space
    float depth = texelFetch(u_gDepth, coord, 0).r;
    vec3 view_pos_reconstructed = ReconstructViewPosition(uv, depth, u_ProjectionInv);

    // Test error of position
    vec3 error = abs(beginPos - view_pos_reconstructed);
    vec4 dbgL = vec4(error * 100, 1);

    // Ray march hiz
    // beginPos = view_pos_reconstructed;
    vec3 viewPos = vec3(0);
    vec3 viewDir = beginPos - viewPos;
    vec3 normal = texelFetch(u_gNormal, coord, 0).xyz;
    vec3 rayDir = normalize(reflect(viewDir, normal));
    
    vec2 hitUV;
    // vec4 hit_uv_L = vec4(texture(u_gPosition, hitUV).rgb, 1);
    // bool naive_ok = NaiveRayMarching(beginPos, rayDir, hitUV);
    // vec4 L = naive_ok ? vec4(texture(u_gAlbedo, hitUV).rgb, 1) : vec4(0, 0, 0, 1);

// RayMarchingHiZ(vec3 beginPos, vec3 rayDir, mat4 camProj, ivec2 screenSize, float maxDistance, float stride, float thickNess, int mipLevelCount, vec2 hitUV, out vec4 debugL)
    vec4 debugL;
    bool hiz_ok = RayMarchingHiZ(beginPos, rayDir, u_Projection, siz, 10.0, 1.0, 0.1, u_MipLevelTotal, hitUV, debugL);
    // vec4 L = hiz_ok ? vec4(texture(u_gAlbedo, hitUV).rgb, 1) : vec4(0, 0, 0, 1);
    vec4 L = hiz_ok ? vec4(1, 1, 1, 1) : vec4(0, 0, 1, 1);

    imageStore(outputImage, coord, debugL);
}