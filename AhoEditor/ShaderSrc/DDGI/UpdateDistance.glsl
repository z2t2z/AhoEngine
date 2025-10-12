#type compute
#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "DDGICommon.glsl"
#include "DDGIHelpers.glsl"
#include "Lighting.glsl"
#include "../PathTracing/PathTracingCommon.glsl"
#include "../PathTracing/IntersectionTest.glsl"
#include "../PathTracing/DataStructs.glsl"

layout(binding = 0, rgba16f) uniform image2D ProbeDistance;
layout(binding = 1, rgba16f) uniform image2D ProbeRadianceDistance;

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;

#define MIN_WEIGHT 0.01

void UpdateBorders(ivec2 localCoords, ivec2 baseCoords, ivec2 fromCoords, vec4 value, const int TEXSIZE);

layout(local_size_x=DDGI_DISTANCE_TEXSIZE, local_size_y=DDGI_DISTANCE_TEXSIZE, local_size_z=1) in;
void main() {
    const int TEXSIZE = DDGI_DISTANCE_TEXSIZE;
    const DDGIVolumeDescGPU volume = u_DDGIVolumeDesc;
    // int probeIndex = int(gl_GlobalInvocationID.x);
    int probeIndex = int(gl_WorkGroupID.x);
    ivec3 probeCoords = GetProbeCoords(probeIndex, volume);
    ivec2 baseTexelCoords = GetProbeTexel(volume, probeCoords, TEXSIZE);

    ivec2 localID = ivec2(gl_LocalInvocationID); 
    ivec2 texelCoords = baseTexelCoords + localID;

    vec2 uv = (vec2(localID) + 0.5) / vec2(TEXSIZE);
    vec3 currDir = OctahedronDecode(uv * 2.0 - 1.0);

    float weightSum = 0.;
    vec2 result = vec2(0);

    for (int i = 0; i < volume.raysPerProbe; i++) {
        int rayIndex = i;
        float dist = imageLoad(ProbeRadianceDistance, ivec2(rayIndex, probeIndex)).a;
        if (dist == -2.0) {
            continue; // Skip backface hits
        }        
        dist = min(abs(dist), length(vec3(volume.probeSpacing, volume.probeSpacing, volume.probeSpacing)) * 1.5);
        vec3 rayDir = DDGIGetProbeRayDirection(rayIndex, volume);
        rayDir = normalize(rayDir); // Ensure direction is normalized

        float weight = max(0, (dot(rayDir, currDir)));
        const float blendExponent = 64;
        weight = pow(weight, blendExponent);
        weightSum += weight;
        result += vec2(abs(dist), dist * dist) * weight;
    }
    
    float eps = 1e-9 * float(volume.raysPerProbe);
    result *= 1.0 / (2.0 * max(weightSum, eps));

    // --- Blending ---
    float blendWeight = 1.0 - volume.hysteresis;
    vec2 prevDist = imageLoad(ProbeDistance, texelCoords).xy;
    if (dot(result, prevDist) == 0.0) {
        blendWeight = 1.0;
    }
    result = mix(prevDist, result, blendWeight);
    imageStore(ProbeDistance, texelCoords, vec4(result.x, result.y, 0.0, 1.0));

    // return;

    UpdateBorders(localID, baseTexelCoords, texelCoords, vec4(result.x, result.y, 0.0, 1.0), TEXSIZE);
}
/*
static const uint4 BORDER_OFFSETS[BORDER_TEXELS] = 
{
	uint4(14, 14, 0, 0), 
	uint4(14, 1, 1, 0), uint4(13, 1, 2, 0), uint4(12, 1, 3, 0), uint4(11, 1, 4, 0), uint4(10, 1, 5, 0), uint4(9, 1, 6, 0), uint4(8, 1, 7, 0), 
	uint4(7, 1, 8, 0), uint4(6, 1, 9, 0), uint4(5, 1, 10, 0), uint4(4, 1, 11, 0), uint4(3, 1, 12, 0), uint4(2, 1, 13, 0), uint4(1, 1, 14, 0),
	uint4(1, 14, 15, 0),
	uint4(1, 14, 0, 1), uint4(1, 13, 0, 2), uint4(1, 12, 0, 3), uint4(1, 11, 0, 4), uint4(1, 10, 0, 5), uint4(1, 9, 0, 6), uint4(1, 8, 0, 7), 
	uint4(1, 7, 0, 8), uint4(1, 6, 0, 9), uint4(1, 5, 0, 10), uint4(1, 4, 0, 11), uint4(1, 3, 0, 12), uint4(1, 2, 0, 13), uint4(1, 1, 0, 14),

	uint4(14, 1, 0, 15), 
	uint4(14, 14, 15, 1), uint4(14, 13, 15, 2), uint4(14, 12, 15, 3), uint4(14, 11, 15, 4), uint4(14, 10, 15, 5), uint4(14, 9, 15, 6), uint4(14, 8, 15, 7), 
	uint4(14, 7, 15, 8), uint4(14, 6, 15, 9), uint4(14, 5, 15, 10), uint4(14, 4, 15, 11), uint4(14, 3, 15, 12), uint4(14, 2, 15, 13), uint4(14, 1, 15, 14),

	uint4(1, 1, 15, 15), 
	uint4(14, 14, 1, 15), uint4(13, 14, 2, 15), uint4(12, 14, 3, 15), uint4(11, 14, 4, 15), uint4(10, 14, 5, 15), uint4(9, 14, 6, 15), uint4(8, 14, 7, 15), 
	uint4(7, 14, 8, 15), uint4(6, 14, 9, 15), uint4(5, 14, 10, 15), uint4(4, 14, 11, 15), uint4(3, 14, 12, 15), uint4(2, 14, 13, 15), uint4(1, 14, 14, 15),
};

*/

void UpdateBorders(ivec2 localCoords, ivec2 baseCoords, ivec2 fromCoords, vec4 value, const int TEXSIZE) {
    // Lower left corner
    if (localCoords.x == 0 && localCoords.y == 0) {
        imageStore(ProbeDistance, fromCoords + ivec2(TEXSIZE, TEXSIZE), value);
    }
    // lower right corner
    if (localCoords.x == TEXSIZE - 1 && localCoords.y == 0) {
        imageStore(ProbeDistance, fromCoords + ivec2(-TEXSIZE, TEXSIZE), value);
    }
    // upper left corner
    if (localCoords.x == 0 && localCoords.y == TEXSIZE - 1) {
        imageStore(ProbeDistance, fromCoords + ivec2(TEXSIZE, -TEXSIZE), value);
    }
    // upper right corner
    if (localCoords.x == TEXSIZE - 1 && localCoords.y == TEXSIZE - 1) {
        imageStore(ProbeDistance, fromCoords + ivec2(-TEXSIZE, -TEXSIZE), value);
    }

    // Bottom row 
    if (localCoords.y == 0) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE - localCoords.x - 1;
        toCoords.y -= 1;
        imageStore(ProbeDistance, toCoords, value); // left
    }
    // Top row
    if (localCoords.y == TEXSIZE - 1) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE - localCoords.x - 1;
        toCoords.y += TEXSIZE;
        imageStore(ProbeDistance, toCoords, value); // right
    }
    // Left column
    if (localCoords.x == 0) {
        ivec2 toCoords = baseCoords;
        toCoords.x -= 1;
        toCoords.y += TEXSIZE - localCoords.y - 1;
        imageStore(ProbeDistance, toCoords, value); // bottom
    }
    // Right column
    if (localCoords.x == TEXSIZE - 1) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE;
        toCoords.y += TEXSIZE - localCoords.y - 1;
        imageStore(ProbeDistance, toCoords, value); // top
    }
}