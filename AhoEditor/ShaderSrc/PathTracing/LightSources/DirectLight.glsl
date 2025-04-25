#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "InfiniteLight.glsl"
#include "../GlobalVars.glsl"
#include "../DataStructs.glsl"
#include "../DisneyPrincipled.glsl"
#include "../Disney.glsl"
#include "../../UniformBufferObjects.glsl"

vec3 SampleDirectLight(State state, const Ray ray) {
    float EnvPdf = 0.0;
    vec3 EnvDir = vec3(0.0);
    vec3 Ld = SampleIBL(EnvPdf, EnvDir);

    if (EnvPdf != 0.0 && VisibilityTest(state.pos, state.pos + 1000000.0 * EnvDir)) {
        vec3 L = EnvDir;
        vec3 V = -ray.direction;
        state.cosTheta = dot(L, V);
        bool front_side = state.cosTheta > 0.0;
        vec3 N = front_side ? state.N : -state.N;
        mat3 tbn = ConstructTBN(N);
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);

        bool is_reflect = SameHemisphere(L, V);
        float eta_path = !front_side ? state.eta : 1.0 / state.eta;
        vec3 H = is_reflect ? normalize(L + V) : normalize(L + V * eta_path);

        float pdf = 0.0;
        vec3 f = principled_eval(state, V, H, L, pdf);
        if (pdf == 0.0) {
            return vec3(0.0);
        }
        float misWeight = PowerHeuristicPdf(EnvPdf, pdf); // Mis weighted
        if (misWeight > 0.0) {
            return misWeight * f * Ld / EnvPdf;
        }
    }

    return vec3(0.0);
}

#endif