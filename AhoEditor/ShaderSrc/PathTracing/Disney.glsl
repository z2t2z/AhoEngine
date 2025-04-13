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
// Return pow(1 - u, 5)
float SchlickFresnel(float u) {
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

/* ========== Diffuse + Subsurface ========== */
vec3 _DisneyDiffuse(const State state, const DotProducts dp, vec3 V, vec3 L, vec3 N, out float pdf) {
    pdf = 0.0;
    pdf = CosineHemispherePDF(L);

    float Fd90 = 0.5 + 2 * state.roughness * Sqr(dp.LdotH);
    return state.baseColor * InvPI * (1.0 + (Fd90 - 1.0) * SchlickFresnel(dp.LdotN)) 
                                   * (1.0 + (Fd90 - 1.0) * SchlickFresnel(dp.VdotN))
                                   * abs(dp.LdotN);
}
vec3 _DisneySubsurface(State state, DotProducts dp, vec3 V, vec3 L, vec3 N) {
    float Fss90 = state.roughness * Sqr(dp.LdotH);
    float FssL = 1.0 + (Fss90 - 1.0) * SchlickFresnel(dp.LdotN);
    float FssV = 1.0 + (Fss90 - 1.0) * SchlickFresnel(dp.VdotN);
    float tmp = 1.0 / (abs(dp.LdotN) + abs(dp.VdotN)) - 0.5;
    return state.baseColor * 1.25 * InvPI * (FssL * FssV * tmp + 0.5) * abs(dp.LdotN);
}
// Diffuse + Subsurface + Sheen
vec3 DisneyDiffuse(State state, vec3 V, out vec3 L, vec3 N, out float pdf) {
    mat3 tbn = ConstructTBN(N);
    V = WorldToLocal(V, tbn);
    L = SampleCosineHemisphere();
    vec3 H = normalize(L + V);
    
    DotProducts dp;
    SetDotProducts(L, V, H, N, dp);

    vec3 BaseDiffuse = _DisneyDiffuse(state, dp, V, L, Y, pdf);
    vec3 Subsurface = _DisneySubsurface(state, dp, V, L, Y);
    
    float lum = Luminance(state.baseColor);
    vec3 Ctint = lum > 0.0 ? state.baseColor / lum : vec3(1.0);
    vec3 Csheen = (1.0 - state.sheenTint) + state.sheenTint * Ctint;
    vec3 fsheen = Csheen * (SchlickFresnel(abs(dp.LdotH))) * abs(dp.LdotN);
    
    vec3 f = (1.0 - state.subsurface) * BaseDiffuse + Subsurface * state.subsurface + fsheen;
    f = BaseDiffuse;

    L = LocalToWorld(L, tbn);
    return f * abs(L.y);
}

/* ========== Metallic Specular ========== */
// Does not include the fresnel term!!
float _DisneySpecularEval(State state, vec3 V, vec3 L, vec3 H, vec3 N) {
    // vec3 Fm = state.baseColor * (1 - state.baseColor) * SchlickFresnel(dot(L, H));
    // float Fm = mix(0.04, 1.0, SchlickFresnel(VdotH));
    float Dm = D_GGX(H, state.ax, state.ay);
    float Gm = G1(V, state.ax, state.ay) * G1(L, state.ax, state.ay);
    return Dm * Gm / (4.0 * dot(V, N));
}
vec3 DisneySpecular(State state, vec3 V, in out vec3 L, vec3 N, out float pdf) {
    mat3 tbn = ConstructTBN(N);
    V = normalize(WorldToLocal(V, tbn));
    vec3 H = SampleGGXVNDF(V, state.ax, state.ay, rand(), rand());
    L = reflect(-V, H);

    DotProducts dp;
    SetDotProducts(L, V, H, N, dp);

    float Fm = mix(0.04, 1.0, SchlickFresnel(dp.VdotH));
    vec3 f = vec3(Fm) * _DisneySpecularEval(state, V, L, H, Y);

    pdf = GGXVNDFLPdf(V, H, L, state.ax, state.ay);

    L = LocalToWorld(L, tbn);
    return f * abs(L.y);
}

/* ========== Clearcoat ========== */
// GTR1 clearcoat ndf
float Dclearcoat(float alpha, vec3 H) {
    if (alpha >= 1.0) {
        return InvPI;
    }
    float a2 = Sqr(alpha);
    float denom = PI * log(a2) * (1.0 + (a2 - 1.0) * Sqr(H.y));
    return (a2 - 1.0) / denom;
}
vec3 _DisneyClearcoatEval(State state, vec3 V, vec3 L, vec3 H, vec3 N, float alpha, out float pdf) {
    pdf = 0.0;
    if (L.y <= 0.0) {
        return vec3(0.0);
    }
    float VdotH = dot(V, H);
    float F = mix(0.04, 1.0, SchlickFresnel(VdotH));
    float D = Dclearcoat(alpha, H);
    float G = G1(V, 0.25, 0.25) * G1(L, 0.25, 0.25);
    float denom = 4.0 * max(dot(V, N), 1e-6);
    pdf = D * H.y / (4.0 * VdotH); // jacobian
    return vec3(F) * D * G;// / denom;
}
// clearcoat
vec3 SampleGTR1(float rgh, float r1, float r2) {
    float a = max(0.001, rgh);
    float a2 = a * a;
    float phi = r1 * 2 * PI;
    float cosTheta = sqrt((1.0 - pow(a2, 1.0 - r2)) / (1.0 - a2));
    float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    return vec3(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi);
}
vec3 DisneyClearcoat(State state, vec3 V, in out vec3 L, vec3 N, out float pdf) {
    mat3 tbn = ConstructTBN(N);
    V = normalize(WorldToLocal(V, tbn));

    float alpha = (1.0 - state.clearcoatGloss) * 0.1 + state.clearcoatGloss * 0.001;
    vec3 H = SampleGTR1(alpha, rand(), rand());
    L = reflect(-V, H);
    vec3 brdf = _DisneyClearcoatEval(state, V, L, H, N, alpha, pdf);

    L = LocalToWorld(L, tbn);
    return brdf;
}

/* ========== Glass ========== */
vec3 _RefractionEval(State state, vec3 V, vec3 L, vec3 H, vec3 N, out float pdf) {
    float D = D_GGX(H, state.ax, state.ay);
    float G = G1(V, state.ax, state.ay) * G1(L, state.ax, state.ay);
    float VdotN = dot(V, N);
    float VdotH = dot(V, H);
    float LdotH = dot(L, H);
    float denom = Sqr(LdotH + VdotH * state.eta);

    float jacobian = abs(LdotH) / denom;
    pdf = G * max(0.0, VdotH) * D * jacobian / V.y;
    return pow(state.baseColor, vec3(0.5)) * D * G * abs(VdotH) * jacobian * Sqr(state.eta) / abs(L.y * V.y);
}
vec3 _DisneyGlassEval(State state, bool Reflect, vec3 V, vec3 L, vec3 H, vec3 N, float F, out float pdf) {
    pdf = 0.0;
    bool IsReflection = SameHemisphere(V, L);
    vec3 f;
    if (Reflect) {
        f = vec3(F) * _DisneySpecularEval(state, V, L, H, N);
        pdf = GGXVNDFLPdf(V, H, L, state.ax, state.ay);
        pdf /= F;
    } else {
        f = vec3(1.0 - F) * _RefractionEval(state, V, L, H, N, pdf);
        pdf /= (1.0 - F);
    }
    return f;
}
vec3 DisneyGlass(State state, vec3 V, in out vec3 L, vec3 N, out float pdf) {
    mat3 tbn = ConstructTBN(N);
    V = normalize(WorldToLocal(V, tbn));

    vec3 H = SampleGGXVNDF(V, state.ax, state.ay, rand(), rand());

    float F = DielectricFresnel(abs(dot(V, H)), state.eta);
    float VdotH = dot(V, H);
    // float F = mix(0.04, 1.0, SchlickFresnel(VdotH));
    // Reflection or refraction
    float ReflectP = rand();
    L = ReflectP < F ? reflect(-V, H) : refract(-V, H, state.eta);

    vec3 brdf = _DisneyGlassEval(state, ReflectP < F, V, L, H, N, F, pdf);

    L = LocalToWorld(L, tbn);
    return brdf;
}

#endif