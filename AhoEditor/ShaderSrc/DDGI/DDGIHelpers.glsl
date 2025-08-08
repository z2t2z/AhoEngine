#ifndef DDGI_HELPERS_GLSL
#define DDGI_HELPERS_GLSL

#include "DDGICommon.glsl"

/*
 * Y-up coordinate system
 * x: width, y: height, z: depth
 * Encode: index=x+z*width+y*width*height
 * Decode:  
 *          x = index % width;
 *          y = index / (width * depth);
 *          z = (index % (width * depth)) / width;
 *  
*/

float Min(vec3 v) {
    return min(min(v.x, v.y), v.z);
}
float saturate(float v) {
    return clamp(v, 0.0, 1.0);
}

/**
 * Returns either -1 or 1 based on the sign of the input value.
 * If the input is zero, 1 is returned.
 */
float SignNotZero(float v) {
    return (v >= 0.0f) ? 1.0f : -1.0f;
}
vec2 SignNotZero(vec2 v) {
    return vec2(SignNotZero(v.x), SignNotZero(v.y));
}
vec2 OctahedronEncode(vec3 dir) {
    vec3 n = dir.xzy;
    float l1norm = abs(n.x) + abs(n.y) + abs(n.z);
    vec2 uv = n.xy / l1norm;
    if (n.z < 0) {
        uv = (1.0 - abs(uv.yx)) * SignNotZero(uv);
    }
    return uv;
}
vec3 OctahedronDecode(vec2 uv) {
    vec3 dir = vec3(uv.x, uv.y, 1.0 - abs(uv.x) - abs(uv.y));
    if (dir.z < 0.0f) {
        float oldX = dir.x;
        dir.x = (1.0f - abs(dir.y)) * SignNotZero(oldX);
        dir.y = (1.0f - abs(oldX)) * SignNotZero(dir.y);
    }
    dir = normalize(dir);
    return dir.xzy;
}

vec3 GetRandomRotation() {
    return vec3(0);
}

/**
 * TODO,No relocation or rotation
 */
vec3 DDGIGetProbeWorldPosition(vec3 probeCoords, DDGIVolumeDescGPU volume) {
    return volume.origin + probeCoords * volume.probeSpacing;
}

ivec3 GetProbeCoords(int probeIndex, DDGIVolumeDescGPU volume) {
    int width = volume.probeCounts.x;
    int height = volume.probeCounts.y;
    int depth = volume.probeCounts.z;

    int x = probeIndex % width;
    int y = probeIndex / (width * depth);
    int z = (probeIndex % (width * depth)) / width; // ???
    return ivec3(x, y, z);
}
ivec2 GetProbeTexel(DDGIVolumeDescGPU volume, ivec3 probeCoords, int numProbeInteriorTexels) {
	int numProbeTexels = numProbeInteriorTexels + 2;
	return ivec2(probeCoords.x + probeCoords.z * volume.probeCounts.x, probeCoords.y) * numProbeTexels + ivec2(1);
}
vec2 GetProbeUV(const DDGIVolumeDescGPU volume, ivec3 probeCoords, vec3 direction, int numProbeInteriorTexels) {
    int numProbeTexels = numProbeInteriorTexels + 2;
    int textureWidth = numProbeTexels * volume.probeCounts.x * volume.probeCounts.z;
    int textureHeight = numProbeTexels * volume.probeCounts.y;

    vec2 pixel = GetProbeTexel(volume, probeCoords, numProbeInteriorTexels);
    vec2 uv = OctahedronEncode(direction) * 0.5 + 0.5;
    pixel += uv * numProbeInteriorTexels;
    
    return pixel / vec2(textureWidth, textureHeight);    
}

/**
 * Computes a low discrepancy spherically distributed direction on the unit sphere,
 * for the given index in a set of samples. Each direction is unique in
 * the set, but the set of directions is always the same.
 */
vec3 SampleSphericalFibonacci(float i, float n) {
#ifdef ADRIA
    float sampleIndex = i;
    float numSamples = n;
    const float b = (sqrt(5.f) * 0.5f + 0.5f) - 1.f;
    float phi = TwoPI * fract(sampleIndex * b);
    float cosTheta = 1.f - (2.f * sampleIndex + 1.f) * (1.f / numSamples);
    float sinTheta = sqrt(saturate(1.f - (cosTheta * cosTheta)));
    return vec3((cos(phi) * sinTheta), cosTheta, (sin(phi) * sinTheta));
#else
	const float PHI = sqrt(5) * 0.5f + 0.5f;
	float fraction = (i * (PHI - 1)) - floor(i * (PHI - 1));
	float phi = 2.0f * PI * fraction;
	float cosTheta = 1.0f - (2.0f * i + 1.0f) * (1.0f / n);
	float sinTheta = sqrt(saturate(1.0 - cosTheta * cosTheta));
	// return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
	return vec3(cos(phi) * sinTheta, cosTheta, sin(phi) * sinTheta);
#endif
}

/**
 * Computes a spherically distributed, normalized ray direction for the given ray index in a set of ray samples.
 * Applies the volume's random probe ray rotation transformation to "non-fixed" ray direction samples.
 */
vec3 DDGIGetProbeRayDirection(int rayIndex, const DDGIVolumeDescGPU volume, mat3 randomRotation = mat3(1.0)) {
    float i = float(rayIndex);
    float N = float(volume.raysPerProbe);
    vec3 sampledDir = SampleSphericalFibonacci(i, N);
    return normalize(sampledDir);
    return normalize(volume.randomRotation * sampledDir); // Apply random rotation
}

vec3 ComputeBias(const DDGIVolumeDescGPU volume, vec3 normal, vec3 viewDir, float bias = 0.2) {
	const float normalBiasMultiplier = 0.2f;
	const float viewBiasMultiplier = 0.8f;
	const float axialDistanceMultiplier = 0.75f;
	// return (normal * normalBiasMultiplier + viewDir * viewBiasMultiplier) * axialDistanceMultiplier * Min(volume.probeSpacing) * bias;
	return (normal * normalBiasMultiplier + viewDir * viewBiasMultiplier) * axialDistanceMultiplier * volume.probeSpacing * bias;    
}

/**
 * https://github.com/mateeeeeee/Adria/blob/3df225848812f6379f877f39924ded16722aaf4d/Adria/Resources/Shaders/DDGI/DDGICommon.hlsli#L103
 * Samples the irradiance at the given world position and normal from the DDGI volume.
 * The direction is assumed to be in world space.
 * Wo: camera to shading point
 */
vec3 SampleDDGIIrradiance(const DDGIVolumeDescGPU volume, vec3 worldPosition, vec3 worldNormal, vec3 Wo, bool gammaCorrect = true) {
    vec3 position = worldPosition;
    vec3 unbiasedPosition = position;
    vec3 direction = normalize(worldNormal);
    float volumeWeight = 1.0; // Controls fade out, when shading point is outside the volume

    vec3 relativeCoords = (position - volume.origin) / volume.probeSpacing;
    for (int i = 0; i < 3; ++i) {
        volumeWeight *= mix(0, 1, saturate(relativeCoords[i]));//clamp(v,0,1) <==> saturate(v)
        if (relativeCoords[i] > volume.probeCounts[i] - 2) { //Need at least 2 probes for interpolation
            float x = saturate(relativeCoords[i] - volume.probeCounts[i] + 2);
            volumeWeight *= mix(1.0, 0.0, x);
        }
    }

    if (volumeWeight <= 0.0) 
        return vec3(0, 0, 0);

    position += ComputeBias(volume, direction, -Wo, 0.2); 

    ivec3 baseProbeCoords = ivec3(floor(relativeCoords));
    vec3 baseProbePosition = DDGIGetProbeWorldPosition(baseProbeCoords, volume);
    vec3 alpha = clamp((position - baseProbePosition) / volume.probeSpacing, vec3(0), vec3(1)); // !

    vec3 sumIrradiance = vec3(0);
    float sumWeight = 0;

    vec3 testIrradiance = vec3(0);

    for (int i = 0; i < 8; ++i) {
        ivec3 indexOffset = ivec3(i, i >> 1, i >> 2) & ivec3(1, 1, 1);
        
        ivec3 probeCoords = clamp(baseProbeCoords + indexOffset, ivec3(0), volume.probeCounts - 1);
        vec3 probePosition = DDGIGetProbeWorldPosition(probeCoords, volume);

        vec3 relativeProbePosition = position - probePosition;
        vec3 probeDirection = normalize(relativeProbePosition);

        vec3 trilinear = max(vec3(0.001), mix(1.0 - alpha, alpha, indexOffset));
        float trilinearWeight = trilinear.x * trilinear.y * trilinear.z;

        float weight = 1;
        
    #if 0
        // weight *= saturate(dot(probeDirection, direction));
    #else
        // NVIDIA Implementation
        const float wrapShading = dot(normalize(probePosition - unbiasedPosition), direction) * 0.5f + 0.5f;
        // weight *= (wrapShading * wrapShading) + 0.2f;        
    #endif

        vec2 distanceUV = GetProbeUV(volume, probeCoords, -probeDirection, DDGI_DISTANCE_TEXSIZE);
        float probeDistance = length(relativeProbePosition);

        vec2 moments = textureLod(volume.DDGIDistance, distanceUV, 0).xy;
        float variance = abs(moments.x * moments.x - moments.y);
        float chebyshev = 1.0;
        if (probeDistance > moments.x) {
            float mD = probeDistance - moments.x;
            chebyshev = variance / (variance + (mD * mD));
            chebyshev = max(chebyshev * chebyshev * chebyshev, 0.0);
        }
        // weight *= max(0.05f, chebyshev);
        weight = max(0.000001f, weight);

        const float crushThreshold = 0.2;
        if (weight < crushThreshold) {
            // weight *= (weight * weight) * (1.0 / (crushThreshold * crushThreshold));
        }

        weight *= trilinearWeight;

        vec2 uv = GetProbeUV(volume, probeCoords, direction, DDGI_IRRADIANCE_TEXSIZE);
        vec3 irradiance = textureLod(volume.DDGIIrradiance, uv, 0).rgb;
        if (gammaCorrect)
            irradiance = pow(irradiance, vec3(2.5));

        testIrradiance += irradiance;

        sumIrradiance += irradiance * weight;
        sumWeight += weight;
    }

    // return testIrradiance * 0.1;                                                                                                                                          
    if (sumWeight == 0) 
        return vec3(0, 0, 0);

    sumIrradiance *= (1.0 / sumWeight);
    sumIrradiance *= sumIrradiance;
    sumIrradiance *= TwoPI;
    sumIrradiance *= volumeWeight;
    return sumIrradiance;
}

#endif