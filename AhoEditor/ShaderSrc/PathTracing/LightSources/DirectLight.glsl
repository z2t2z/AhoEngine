#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "InfiniteLight.glsl"
#include "AreaLight.glsl"
#include "../IntersectionTest.glsl"
#include "../DisneyPrincipled.glsl"

vec3 SampleDirectLight(State state, const Ray ray);
vec3 _SampleEnvironmentLight(State state, const Ray ray);
// vec3 _SamplePointLight(State state, const Ray ray);
// vec3 _SampleAreaLight(State state, const Ray ray);
vec3 _SampleDirectionalLight(State state, const Ray ray);


vec3 SampleDirectLight(State state, const Ray ray) {
    vec3 Ltot = vec3(0.0);
    Ltot += _SampleEnvironmentLight(state, ray);
    Ltot += _SampleDirectionalLight(state, ray);
    return Ltot;
}

vec3 _SampleEnvironmentLight(State state, const Ray ray) {
    vec3 Ltot = vec3(0.0);
#ifdef FEATURE_ENABLE_IBL    
    float sampledPdf = 0.0;
    vec3 sampledDir = vec3(0.0);
    vec3 Ld = SampleIBL(sampledPdf, sampledDir);
    Ray shadowRay;
    shadowRay.origin = state.pos + 0.001 * sampledDir; // Avoid self-occluded
    shadowRay.direction = sampledDir;
    if (sampledPdf > 0.0 && !IsOccluded(shadowRay, FLT_MAX)) {
        vec3 L = sampledDir;
        vec3 V = -ray.direction;
        state.cosTheta = dot(L, state.N);
        bool front_side = state.cosTheta > 0.0;
        vec3 N = front_side ? state.N : -state.N;
        mat3 tbn = ConstructTBN(N);
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);

        float bsdfPdf = 0.0;
        vec3 f = principled_eval(state, V, L, bsdfPdf);
        if (bsdfPdf > 0.0) {
            float misWeight = PowerHeuristicPdf(sampledPdf, bsdfPdf); // Mis weighted
            if (misWeight > 0.0) {
                Ltot += misWeight * f * Ld / sampledPdf;
            }
        }
    }
#else
    if (uniformSky == vec3(0.0))
        return Ltot;
    
    float sampledPdf = Inv4PI; 
    vec3 Ld = uniformSky * EnvIntensity;
    vec3 sampledDir = SampleUniformSphere();
    Ray shadowRay;
    shadowRay.origin = state.pos + 0.001 * sampledDir; // Avoid self-occluded
    shadowRay.direction = sampledDir;
    if (!IsOccluded(shadowRay, FLT_MAX)) {
        vec3 L = sampledDir;
        vec3 V = -ray.direction;
        state.cosTheta = dot(L, state.N);
        bool front_side = state.cosTheta > 0.0;
        vec3 N = front_side ? state.N : -state.N;
        mat3 tbn = ConstructTBN(N);
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);

        float bsdfPdf = 0.0;
        vec3 f = principled_eval(state, V, L, bsdfPdf);
        if (bsdfPdf > 0.0) {
            float misWeight = PowerHeuristicPdf(sampledPdf, bsdfPdf); // Mis weighted
            if (misWeight > 0.0) {
                Ltot += misWeight * f * Ld / sampledPdf;
            }
        }
    }
#endif
    return Ltot;
}

// TODO
vec3 _SampleDirectionalLight(State state, const Ray ray) {
    const int MAX_LIGHT_CNT = 4;
    vec3 Ltot = vec3(0);
    vec3 N = state.N;
    for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirectionalLight[i];
        if (light.intensity == 0.0) 
            continue;
        
        vec3 L = light.direction; // Note facing direction
        float LoN = dot(L, N);
        if (LoN < 0.0) 
            continue;

        Ray shadowRay;
        shadowRay.origin = state.pos + 0.001 * L; // Avoid self-occluded
        shadowRay.direction = L;
        if (IsOccluded(shadowRay, FLT_MAX))
            continue;

        state.cosTheta = LoN;
        bool front_side = state.cosTheta > 0.0;
        vec3 N = front_side ? state.N : -state.N;
        mat3 tbn = ConstructTBN(N);
        vec3 V = -ray.direction;
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);        
        float bsdfPdf = 0.0;
        float sampledPdf = 1.0; //TODO: Why is it 1. ?
        vec3 f = principled_eval(state, V, L, bsdfPdf);
        if (bsdfPdf > 0.0) {
            float misWeight = PowerHeuristicPdf(sampledPdf, bsdfPdf); // Mis weighted
            vec3 Ld = light.color * light.intensity;
            if (misWeight > 0.0) {
                Ltot += misWeight * f * Ld / sampledPdf;
            }
        }
    }
    return Ltot;
}

vec4 EvaluateInfiniteLight(const Ray ray) {
#ifdef FEATURE_ENABLE_IBL
    if (u_EnvMap.EnvSize.x == 0 || u_EnvMap.EnvSize.y == 0) {
        return vec4(EnvIntensity * uniformSky, -1.0f);
    }
    return EvalEnvMap(ray.direction);
#else
    return vec4(uniformSky, Inv4PI);
#endif
}

#endif