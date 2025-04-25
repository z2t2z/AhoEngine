#ifndef SAMPLING_GLSL
#define SAMPLING_GLSL

#include "Random.glsl"
#include "GlobalVars.glsl"

vec3 SampleCosineHemisphere() {
    vec2 u = vec2(rand(), rand());
    float r = sqrt(u.x);
    float phi = 2.0f * PI * u.y;
    float x = r * cos(phi);
    float z = r * sin(phi);
    float y = sqrt(max(0.0, 1.0 - x * x - z * z));
    return vec3(x, y, z);
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

// https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
float PowerHeuristicPdf(float pdfA, float pdfB) {
    return (pdfA * pdfA) / ((pdfA * pdfA) + (pdfB * pdfB));
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

// Mituba's implememtation
float fresnel(float cos_theta_i, float eta) {
    float sinThetaTSq = eta * eta * (1.0f - cos_theta_i * cos_theta_i);

    // Total internal reflection
    if (sinThetaTSq > 1.0)
        return 1.0;

    float cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

    float rs = (eta * cosThetaT - cos_theta_i) / (eta * cosThetaT + cos_theta_i);
    float rp = (eta * cos_theta_i - cosThetaT) / (eta * cos_theta_i + cosThetaT);

    return 0.5f * (rs * rs + rp * rp);

    // bool outside_mask = cos_theta_i >= 0.0;
    // float rcp_eta = 1.0 / eta;
    // float eta_it  = outside_mask ? eta  : rcp_eta;
    // float eta_ti  = outside_mask ? rcp_eta : eta;
    // float cos_theta_t_sqr = 
    //     1.0 - (eta_ti * eta_ti) * (1.0 - cos_theta_i * cos_theta_i);
    // float cos_theta_i_abs = abs(cos_theta_i);
    // float cos_theta_t_abs = sqrt(max(0.0, cos_theta_t_sqr));
    // bool index_matched = (eta == 1.0);
    // bool special_case  = index_matched || (cos_theta_i_abs == 0.0);
    // float r_sc = index_matched ? 0.0 : 1.0;
    // float a_s = (eta_it * cos_theta_t_abs - cos_theta_i_abs) /
    //             (eta_it * cos_theta_t_abs + cos_theta_i_abs);
    // float a_p = (eta_it * cos_theta_i_abs - cos_theta_t_abs) /
    //             (eta_it * cos_theta_i_abs + cos_theta_t_abs);

    // float r = 0.5 * (a_s * a_s + a_p * a_p);
    // if (special_case) {
    //     r = r_sc;
    // }
    // return r;
}

// Return pow(1 - u, 5)
float SchlickFresnel(float u) {
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}
// Return pow(1 - u, 5)
float SchlickWeight(float u) {
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5) 
}

#endif