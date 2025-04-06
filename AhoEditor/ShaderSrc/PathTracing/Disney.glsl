#ifndef DISNEY_GLSL
#define DISNEY_GLSL

#include "./Math.glsl"
#include "./Sampling.glsl"
#include "./Random.glsl"
#include "./DataStructs.glsl"

#include "./Sampling/GGXSampling.glsl"

/*
    Mostly based on homework1 of https://cseweb.ucsd.edu/~tzli/cse272/wi2024/
*/
float SchlickFresnel(float u) {
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}
vec3 _DisneyDiffuse(State state, vec3 V, vec3 L, vec3 N) {
    float LdotN = dot(L, N);
    float VdotN = dot(V, N);
    float LdotH = dot(L, normalize(L + V));
    float Fd90 = 0.5 + 2 * state.roughness * LdotH * LdotH;
    return state.baseColor * InvPI * (1.0 + (Fd90 - 1.0) * SchlickFresnel(LdotN)) 
                                   * (1.0 + (Fd90 - 1.0) * SchlickFresnel(VdotN));
}
vec3 _DisneySubsurface(State state, vec3 V, vec3 L, vec3 N) {
    float LdotN = dot(L, N);
    float VdotN = dot(V, N);
    float LdotH = dot(L, normalize(L + V));
    float Fss90 = state.roughness * LdotH * LdotH;
    float FssL = 1.0 + (Fss90 - 1.0) * SchlickFresnel(LdotN);
    float FssV = 1.0 + (Fss90 - 1.0) * SchlickFresnel(VdotN);
    float tmp = 1.0 / (abs(LdotN) + abs(VdotN)) - 0.5;
    return state.baseColor * 1.25 * InvPI * (FssL * FssV * tmp + 0.5) * abs(LdotN);
}
vec3 DisneyDiffuse(State state, vec3 V, out vec3 L, vec3 N, out float pdf) {
    pdf = 0.0;
    mat3 tbn = ConstructTBN(N);
    V = WorldToLocal(V, tbn);
    L = SampleCosineHemisphere();
    pdf = CosineHemispherePDF(L);
    vec3 baseDiffuse = _DisneyDiffuse(state, V, L, Y);
    vec3 subsurface = _DisneySubsurface(state, V, L, Y);
    vec3 brdf = (1.0 - state.subsurface) * baseDiffuse + subsurface * state.subsurface;
    L = LocalToWorld(L, tbn);
    return brdf;
}


vec3 _DisneySpecularEval(State state, vec3 V, vec3 L, vec3 N) {
    vec3 H = normalize(V + L);
    vec3 Fm = state.baseColor * (1 - state.baseColor) * SchlickFresnel(dot(L, H));
    float Dm = D_GGX(H, state.ax, state.ay);
    float Gm = G1(V, state.ax, state.ay) * G1(L, state.ax, state.ay);
    return Fm * Dm * Gm / (4.0 * dot(V, N));
}
vec3 DisneySpecular(State state, vec3 V, in out vec3 L, vec3 N, out float pdf) {
    mat3 tbn = ConstructTBN(N);
    V = normalize(WorldToLocal(V, tbn));

    vec3 H = SampleGGXVNDF(V, state.ax, state.ay, rand(), rand());
    L = reflect(-V, H);
    vec3 brdf = _DisneySpecularEval(state, V, L, Y);

    pdf = GGXVNDFLPdf(V, H, L, state.ax, state.ay);

    L = LocalToWorld(L, tbn);
    return brdf;
}

#endif