#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "InfiniteLight.glsl"
#include "../GlobalVars.glsl"
#include "../DataStructs.glsl"
#include "../Disney.glsl"
#include "../../UniformBufferObjects.glsl"

vec3 SampleDirectLight(Ray ray, vec3 hitPos, vec3 N, HitInfo info, vec2 uv, int textureHandleId) {
    vec3 lightPos = vec3(u_LightPosition[0]);
        return vec3(0.0, 0.0, 0.0);
    if (lightPos.y == 0.0) {
    }
    vec3 Ldir = normalize(lightPos - hitPos);
    if (VisibilityTest(hitPos, lightPos)) {
        float NdotL = max(0.0, dot(N, Ldir));
        float attenuation = length(lightPos - hitPos);
        attenuation *= attenuation;
        float I = 1000.0;
        return I * InvPI * NdotL / attenuation * vec3(1.0, 1.0, 1.0);
    }
    return vec3(0.0, 0.0, 0.0);
}

vec3 SampleDirectLight(const State state, const Ray ray) {
    float EnvPdf = 0.0;
    vec3 EnvDir = vec3(0.0);
    vec3 Ld = SampleIBL(EnvPdf, EnvDir);

    if (EnvPdf != 0.0 && VisibilityTest(state.pos, state.pos + 1000000.0 * EnvDir)) {
        vec3 L = EnvDir;
        vec3 V = -ray.direction;
        vec3 N = state.N;
        mat3 tbn = ConstructTBN(N);
        L = WorldToLocal(L, tbn);
        V = WorldToLocal(V, tbn);
        if (!SameHemisphere(L, V)) {
            return vec3(0.0);
        }
        vec3 H = normalize(L + V);

        DotProducts dp;
        SetDotProducts(L, V, H, Y, dp);

        float BsdfPdf = 0.0;
        vec3 f = _DisneyDiffuse(state, dp, V, L, N, BsdfPdf) * abs(L.y); // Evaluate bsdf pdf for this direction
        float misWeight = PowerHeuristicPdf(EnvPdf, BsdfPdf); // Mis weighted
        if (misWeight > 0.0) {
            return misWeight * (f * Ld / EnvPdf);
        }
    }

    return vec3(0.0);
}

#endif