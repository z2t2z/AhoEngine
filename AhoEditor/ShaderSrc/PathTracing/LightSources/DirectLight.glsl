#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "InfiniteLight.glsl"
#include "AreaLight.glsl"
#include "../IntersectionTest.glsl"
#include "../DisneyPrincipled.glsl"

vec3 SampleDirectLight(State state, const Ray ray) {
    vec3 Ltot = vec3(0.0);

#ifdef OPT_INFINITE_LIGHT    
    float envPdf = 0.0;
    vec3 EnvDir = vec3(0.0);
    vec3 Ld = SampleIBL(envPdf, EnvDir);
    if (envPdf != 0.0 && VisibilityTest(state.pos, state.pos + 1000000.0 * EnvDir)) {
        vec3 L = EnvDir;
        vec3 V = -ray.direction;
        state.cosTheta = dot(L, V);
        bool front_side = state.cosTheta > 0.0;
        vec3 N = front_side ? state.N : -state.N;
        mat3 tbn = ConstructTBN(N);
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);

        float bsdfPdf = 0.0;
        vec3 f = principled_eval(state, V, L, bsdfPdf);
        if (bsdfPdf > 0.0) {
            float misWeight = PowerHeuristicPdf(envPdf, bsdfPdf); // Mis weighted
            if (misWeight > 0.0) {
                Ltot += misWeight * f * Ld / envPdf;
            }
        }
    }
#endif

    // Sample area lights
    // state.cosTheta = dot(state.N, -ray.direction); // cosTheta changed during ibl sampling, so set it again
    // uint areaLightCount = u_LightCount.a;
    // float choosenPdf = areaLightCount == 0u ? 1.0 : 1.0 / float(areaLightCount);
    // for (uint i = 0; i < areaLightCount; ++i) {
    //     Ltot += SampleRectangleAreaLight(state, -ray.direction, u_AreaLight[i], choosenPdf);
    // }

    return Ltot;
}

#endif