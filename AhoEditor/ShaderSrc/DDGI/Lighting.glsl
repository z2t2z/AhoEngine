#ifndef DDGI_LIGHTING
#define DDGI_LIGHTING

#include "DDGICommon.glsl"
#include "../PathTracing/PathTracingCommon.glsl"
#include "../PathTracing/IntersectionTest.glsl"
#include "../PathTracing/DataStructs.glsl"

//V:view direction, L:light direction, all in world space
vec3 _EvaluateDirectionalDiffuse(const Material mat, vec3 pos, vec3 N, vec3 V);
vec3 _EvaluatePointLightDiffuse(const Material mat, vec3 pos, vec3 N, vec3 V);

//V:view direction, L:light direction, all in world space
vec3 EvaluateDirectDiffuse(const Material mat, vec3 pos, vec3 N, vec3 V) {
    vec3 brdf = mat.baseColor * InvPI;
    vec3 lighting = vec3(0);
    lighting += _EvaluateDirectionalDiffuse(mat, pos, N, V);
    return (brdf * lighting);
}

//TODO
vec3 EvaluateEnvironmentDiffuse(vec3 direction) {
    vec3 lighting = vec3(0);
#ifdef FEATURE_ENABLE_IBL
	// lighting += EvalEnvLight(mat, fragPos, F0, V, N);
#else
    const vec3 ambientColor = vec3(0.1f, 0.1f, 0.1f);
    // lighting += ambientColor; // Fallback ambient color
#endif
    return lighting;
}

vec3 _EvaluateDirectionalDiffuse(const Material mat, vec3 pos, vec3 N, vec3 V) {
    vec3 lighting = vec3(0);
    for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirectionalLight[i];
        if (light.intensity <= 0.001) 
            continue;

        vec3 L = light.direction;// Note facing direction
        float LoN = dot(L, N);
        if (LoN <= 0.001) 
            continue;

        // Visibility test
        Ray shadowRay;
        shadowRay.origin = pos + 0.01 * N; // Avoid self-occluded
        shadowRay.direction = L;
        if (IsOccluded(shadowRay, FLT_MAX))
            continue; //TODO: Wrong?
            
        lighting += light.intensity * light.color * LoN;
    }
    return lighting;
}
#endif