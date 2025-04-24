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
// 定义返回类型
struct FresnelResult {
    float  r;            // 能量反射率
    float  cos_theta_t;  // 带符号的折射余弦
    float  eta_it;       // 入射→透射折射率比
    float  eta_ti;       // 透射→入射折射率比
};

// 计算菲涅尔特例的函数
FresnelResult fresnel(float cos_theta_i, float eta) {
    // 1. 内外部判断
    bool outside = (cos_theta_i >= 0.0);
    
    // 2. η 比率
    float rcp_eta = 1.0 / eta;
    float eta_it  = outside ? eta     : rcp_eta;
    float eta_ti  = outside ? rcp_eta : eta;
    
    // 3. Snell 定律，计算 cos²θ_t
    float sin2_i = max(0.0, 1.0 - cos_theta_i*cos_theta_i);
    float sin2_t = eta_ti * eta_ti * sin2_i;
    float cos2_t = max(0.0, 1.0 - sin2_t);
    
    // 4. 绝对值余弦
    float cos_i_abs = abs(cos_theta_i);
    float cos_t_abs = sqrt(cos2_t);
    
    // 5. 特殊情况掩码
    bool index_matched = (eta == 1.0);
    bool grazing       = (cos_i_abs == 0.0);
    bool special_case  = index_matched || grazing;
    float r_sc         = index_matched ? 0.0 : 1.0;
    
    // 6. 计算 s/p 振幅反射系数
    float a_s = (eta_it * cos_t_abs - cos_i_abs) /
                (eta_it * cos_t_abs + cos_i_abs);
    float a_p = (eta_it * cos_i_abs - cos_t_abs) /
                (eta_it * cos_i_abs + cos_t_abs);
    
    // 7. 能量反射率
    float r = 0.5 * (a_s*a_s + a_p*a_p);
    if (special_case) {
        r = r_sc;
    }
    
    // 8. 带符号的折射余弦
    float cos_t = cos_t_abs * (cos_theta_i < 0.0 ? -1.0 : 1.0);
    
    // 9. 返回结果
    FresnelResult result;
    result.r           = r;
    result.cos_theta_t = cos_t;
    result.eta_it      = eta_it;
    result.eta_ti      = eta_ti;
    return result;
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