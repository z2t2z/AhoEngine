#ifndef BXDF_COMMON_GLSL
#define BXDF_COMMON_GLSL

#include "./SSBO.glsl"
#include "./DataStructs.glsl"
#include "./Sampling.glsl"
#include "./Math.glsl"

struct BSDFSample {
    vec3 wi;  
    float f;  
    float pdf;
};


// TrowbridgeReitzDistribution
// https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/microfacet.cpp
float alpha_x = 0.01;
float alpha_y = 0.01;
float alphax = alpha_x;
float alphay = alpha_y;
/* Normal Distribution */
float D(vec3 wh) {  
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta)) {
        return 0.0;
    }
    float cos4Theta = Sqr(Cos2Theta(wh));
    float e = (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) * tan2Theta;
    return 1 / (PI * alpha_x * alpha_y * cos4Theta * Sqr(1 + e));
}
float Lambda(vec3 w) {
    float absTanTheta = abs(TanTheta(w));
    if (isinf(absTanTheta)) {
        return 0.0;
    }
    // Compute _alpha_ for direction _w_
    float alpha =
        sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + sqrt(1.0 + alpha2Tan2Theta)) / 2;
}
float G1(vec3 w) { 
    return 1 / (1 + Lambda(w)); 
}
float G(vec3 wo, vec3 wi) {
    return 1 / (1 + Lambda(wo) + Lambda(wi));
}

// Seems good
vec3 Sample_wh(const vec3 wo) { // Modified!
    vec3 wh;
    vec2 u = vec2(rand(), rand());

    float cosTheta = 0, phi = (2 * PI) * u[1];
    if (alphax == alphay) {
        float tanTheta2 = alphax * alphax * u[0] / (1.0f - u[0]);
        cosTheta = 1 / sqrt(1 + tanTheta2);
    } else {
        phi = atan(alphay / alphax * tan(2 * PI * u[1] + 0.5 * PI));
        if (u[1] > 0.5) {
            phi += PI;
        }
        float sinPhi = sin(phi), cosPhi = cos(phi);
        const float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
        const float alpha2 = 1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
        cosTheta = 1 / sqrt(1 + tanTheta2);
    }
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    wh = SphericalDirection(sinTheta, cosTheta, phi);

    if (!SameHemisphere(wo, wh)) {
        wh = -wh;
    }
    return wh;
}
float Pdf_TR(const vec3 wo, const vec3 wh) {
    return D(wh) * AbsCosTheta(wh);
    return D(wh) * G1(wo) * AbsCosTheta(wh);
}

vec2 SampleUniformDiskPolar() {
    vec2 u = vec2(rand(), rand());
    float r = sqrt(u[0]);
    float theta = 2 * PI * u[1];
    return vec2(r * cos(theta), r * sin(theta));
}

// https://github.com/mmp/pbrt-v4/blob/779d1a78b74aab393853544198189729434121b5/src/pbrt/util/scattering.h#L164
vec3 Sample_wm(vec3 w) {
    vec3 wh = normalize(vec3(alpha_x * w.x, alpha_y * w.y, w.z));
    if (wh.z < 0) {
        wh = -wh;
    }

    vec3 T1 = (wh.z < 0.99999f) ? normalize(cross(vec3(0, 0, 1), wh))
                                    : vec3(1, 0, 0);
    vec3 T2 = cross(wh, T1);

    vec2 p = SampleUniformDiskPolar();

    float h = sqrt(1.0 - Sqr(p.x));
    p.y = mix((1.0 + wh.z) / 2.0, h, p.y);

    float pz = sqrt(max(0.0, 1.0 - LengthSquared(p)));
    vec3 nh = p.x * T1 + p.y * T2 + pz * wh;
    vec3 res = normalize(vec3(alpha_x * nh.x, alpha_y * nh.y, max(1e-6f, nh.z)));
    return res;
}
float D(vec3 w, vec3 wm) {
    return G1(w) / AbsCosTheta(w) * D(wm) * abs(dot(w, wm));
}
float PDF(vec3 w, vec3 wm) { 
    return D(w, wm); 
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

// https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/scattering.h#L61
float FrDielectric(float cosTheta_i, float eta = 1.5) {
    return DielectricFresnel(cosTheta_i, eta);
    cosTheta_i = clamp(cosTheta_i, -1, 1);
    if (cosTheta_i < 0) {
        eta = 1 / eta;
        cosTheta_i = -cosTheta_i;
    }

    float sin2Theta_i = 1 - Sqr(cosTheta_i);
    float sin2Theta_t = sin2Theta_i / Sqr(eta);
    if (sin2Theta_t >= 1) {
        return 1.0;
    }
    float cosTheta_t = sqrt(1 - sin2Theta_t);

    float r_parl = (eta * cosTheta_i - cosTheta_t) / (eta * cosTheta_i + cosTheta_t);
    float r_perp = (cosTheta_i - eta * cosTheta_t) / (cosTheta_i + eta * cosTheta_t);
    return (Sqr(r_parl) + Sqr(r_perp)) / 2;
}

#endif
