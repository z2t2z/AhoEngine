#ifndef SAMPLING_GLSL
#define SAMPLING_GLSL

#include "./Random.glsl"
#include "./GlobalVars.glsl"

vec3 SampleCosineHemisphere() {
    vec2 u = vec2(rand(), rand());
    float r = sqrt(u.x);
    float phi = 2.0f * PI * u.y;
    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(max(0.0, 1.0 - x * x - y * y));
    return vec3(x, z, y);
}

float CosineHemispherePDF(vec3 dir) {
    return dir.y / PI;
}

vec3 SampleUniformHemisphere() {
    vec2 u = vec2(rand(), rand());
    float phi = 2.0f * PI * u.y;
    float cosTheta = 1.0f - u.x;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float z = cosTheta;
    return vec3(x, z, y);
}

float UniformHemispherePDF(vec3 dir = vec3(0.0, 0.0, 0.0)) {
    return Inv2PI;
}

vec3 SampleUniformSphere() {
    vec2 u = vec2(rand(), rand());
    float z = 1 - 2 * u[0];
    float r = sqrt(max(0.f, 1 - z*z));
    float phi = 2 * PI * u[1];
    return vec3(r * cos(phi), r * sin(phi), z);
}

float PowerHeuristicPdf(float pdfA, float pdfB) {
    return Sqr(pdfA) / (Sqr(pdfA) + Sqr(pdfB));
}
float DielectricFresnel(float cosThetaI, float eta) {
    float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);

    // Total internal reflection
    if (sinThetaTSq > 1.0)
        return 1.0;

    float cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

    float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);
    float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

    return 0.5f * (rs * rs + rp * rp);
}
float FresnelDielectric(vec3 L, vec3 V, vec3 H, float ior) {
    float VdotH = dot(V, H);
    float LdotH = dot(L, H);
    float Rs = (VdotH - ior * LdotH) / (VdotH + ior * LdotH);
    float Rp = (ior * VdotH - LdotH) / (ior * VdotH + LdotH);
    return 0.5 * (Sqr(Rs) + Sqr(Rp));
}
#endif