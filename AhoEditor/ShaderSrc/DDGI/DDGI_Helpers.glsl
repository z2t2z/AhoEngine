#ifndef DDGI_HELPERS_GLSL
#define DDGI_HELPERS_GLSL

#include "Common.glsl"

vec3 GetProbeWorldPosition(uvec3 probeCoords, const DDGIVolumeDescGPU volume) {
    // Multiply the grid coordinates by the probe spacing
    vec3 probeGridWorldPosition = probeCoords * volume.probeSpacing;
    return probeGridWorldPosition;
}

/**
 * Computes a low discrepancy spherically distributed direction on the unit sphere,
 * for the given index in a set of samples. Each direction is unique in
 * the set, but the set of directions is always the same.
 */
vec3 SampleSphericalFibonacci(float sampleIndex, float numSamples) {
    const float b = (sqrt(5.0) - 1.0) * 0.5;
    float frac = sampleIndex * b - floor(sampleIndex * b);
    float phi = TwoPI * frac;
    float cosTheta = 1.f - (2.f * sampleIndex + 1.f) * (1.f / numSamples);
    float sinTheta = sqrt(clamp(1.f - (cosTheta * cosTheta), 0.0, 1.0));
    return vec3((cos(phi) * sinTheta), (sin(phi) * sinTheta), cosTheta);
}

vec3 GetProbeRayDirection(uint rayIndex, const DDGIVolumeDescGPU volume) {
    bool isFixedRay = false;
    uint sampleIndex = rayIndex;
    uint numRays = volume.raysPerProbe;

    //TODO: volumn settings
    /*if (volume.probeRelocationEnabled || volume.probeClassificationEnabled)*/

    // Get a ray direction on the sphere
    vec3 direction = SampleSphericalFibonacci(sampleIndex, numRays);

    // Don't rotate fixed rays so relocation/classification are temporally stable
    if (isFixedRay) return normalize(direction);

    return normalize(direction);
    //TODO: random rotation for the sampled direction
    // Apply a random rotation and normalize the direction
    // return normalize(RTXGIQuaternionRotate(direction, RTXGIQuaternionConjugate(volume.probeRayRotation)));
}

uvec3 GetRayDataTexelCoords(uint rayIndex, uint probeIndex, const DDGIVolumeDescGPU volume) {
    uint probesPerPlane = volume.probeCountX * volume.probeCountZ; // Y is up

    uvec3 coords;
    coords.x = rayIndex;
    coords.z = probeIndex / probesPerPlane;
    coords.y = probeIndex - (coords.z * probesPerPlane);
    return coords;
}

#endif