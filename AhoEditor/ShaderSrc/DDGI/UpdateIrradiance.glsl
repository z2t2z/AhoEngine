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

layout(binding = 0, rgba16f) uniform image2D ProbeIrradiance;
layout(binding = 1, rgba16f) uniform image2D ProbeRadianceDistance;

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;

void UpdateBorders(ivec2 localCoords, ivec2 baseCoords, ivec2 fromCoords, vec4 value, const int TEXSIZE);

layout(local_size_x=DDGI_IRRADIANCE_TEXSIZE, local_size_y=DDGI_IRRADIANCE_TEXSIZE, local_size_z=1) in;
void main() {
    const int TEXSIZE = DDGI_IRRADIANCE_TEXSIZE;
    const DDGIVolumeDescGPU volume = u_DDGIVolumeDesc;
    // int probeIndex = int(gl_GlobalInvocationID.x);
    int probeIndex = int(gl_WorkGroupID.x);
    ivec3 probeCoords = GetProbeCoords(probeIndex, volume);
    ivec2 baseTexelCoords = GetProbeTexel(volume, probeCoords, TEXSIZE);

    ivec2 localID = ivec2(gl_LocalInvocationID); 
    ivec2 texelCoords = baseTexelCoords + localID;

    vec2 uv = (vec2(localID) + 0.5) / vec2(TEXSIZE);
    vec3 currDir = OctahedronDecode(uv * 2.0 - 1.0);

    vec3 result = vec3(0);
    float weightSum = 0.0;
    for (int i = 0; i < volume.raysPerProbe; i++) {
        int rayIndex = i;
        float dist = imageLoad(ProbeRadianceDistance, ivec2(rayIndex, probeIndex)).a;
        if (dist <= 0) {
            continue; // Skip backface hits
        }
        vec3 radiance = imageLoad(ProbeRadianceDistance, ivec2(rayIndex, probeIndex)).rgb;
        vec3 rayDir = DDGIGetProbeRayDirection(rayIndex, volume);
        rayDir = normalize(rayDir); // Ensure direction is normalized

        float weight = saturate(dot(rayDir, currDir));
        result += weight * radiance;
        weightSum += weight;
    }
    
    const float eps = 1e-9 * float(volume.raysPerProbe);
    result *= 1.0 / (2.0 * max(weightSum, eps));
    result = pow(result, vec3(0.2));

    // --- Blending ---
    vec3 prevIrradiance = imageLoad(ProbeIrradiance, texelCoords).rgb;
    result = mix(prevIrradiance, result, 1 - volume.hysteresis);
    imageStore(ProbeIrradiance, texelCoords, vec4(result, 1.0));

    // return;

    UpdateBorders(localID, baseTexelCoords, texelCoords, vec4(result, 1.0), TEXSIZE);
}

void UpdateBorders(ivec2 localCoords, ivec2 baseCoords, ivec2 fromCoords, vec4 value, const int TEXSIZE) {
    // Lower left corner
    if (localCoords.x == 0 && localCoords.y == 0) {
        imageStore(ProbeIrradiance, fromCoords + ivec2(TEXSIZE, TEXSIZE), value);
    }
    // lower right corner
    if (localCoords.x == TEXSIZE - 1 && localCoords.y == 0) {
        imageStore(ProbeIrradiance, fromCoords + ivec2(-TEXSIZE, TEXSIZE), value);
    }
    // upper left corner
    if (localCoords.x == 0 && localCoords.y == TEXSIZE - 1) {
        imageStore(ProbeIrradiance, fromCoords + ivec2(TEXSIZE, -TEXSIZE), value);
    }
    // upper right corner
    if (localCoords.x == TEXSIZE - 1 && localCoords.y == TEXSIZE - 1) {
        imageStore(ProbeIrradiance, fromCoords + ivec2(-TEXSIZE, -TEXSIZE), value);
    }

    // Bottom row 
    if (localCoords.y == 0) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE - localCoords.x - 1;
        toCoords.y -= 1;
        imageStore(ProbeIrradiance, toCoords, value); // left
    }
    // Top row
    if (localCoords.y == TEXSIZE - 1) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE - localCoords.x - 1;
        toCoords.y += TEXSIZE;
        imageStore(ProbeIrradiance, toCoords, value); // right
    }
    // Left column
    if (localCoords.x == 0) {
        ivec2 toCoords = baseCoords;
        toCoords.x -= 1;
        toCoords.y += TEXSIZE - localCoords.y - 1;
        imageStore(ProbeIrradiance, toCoords, value); // bottom
    }
    // Right column
    if (localCoords.x == TEXSIZE - 1) {
        ivec2 toCoords = baseCoords;
        toCoords.x += TEXSIZE;
        toCoords.y += TEXSIZE - localCoords.y - 1;
        imageStore(ProbeIrradiance, toCoords, value); // top
    }
}