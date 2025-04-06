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
float D(const vec3 wh) {  
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta)) {
        return 0.0;
    }
    float cos4Theta = Sqr(Cos2Theta(wh));
    float e = (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) * tan2Theta;
    return 1 / (PI * alpha_x * alpha_y * cos4Theta * Sqr(1 + e));
}

float BeckmannDistribution(const vec3 wh) {
    float tan2Theta = Tan2Theta(wh);
    if (isinf(tan2Theta)) {
        return 0.0;
    }
    float cos4Theta = Sqr(Cos2Theta(wh));
    return exp(-tan2Theta * (Cos2Phi(wh) / (alphax * alphax) +
                                  Sin2Phi(wh) / (alphay * alphay))) /
           (PI * alphax * alphay * cos4Theta);
}

float Lambda(vec3 w) {
    float absTanTheta = abs(TanTheta(w));
    if (isinf(absTanTheta)) {
        return 0.0;
    }
    // Compute _alpha_ for direction _w_
    float alpha =
        sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
    float alpha2Tan2Theta = Sqr(alpha * absTanTheta);
    return (-1 + sqrt(1.0 + alpha2Tan2Theta)) / 2;
}
float G1(vec3 w) { 
    return 1 / (1 + Lambda(w)); 
}
float G(vec3 wo, vec3 wi) {
    return 1 / (1 + Lambda(wo) + Lambda(wi));
}
// https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/microfacet.cpp#L238
void TrowbridgeReitzSample11(float cosTheta, float U1, float U2, out float slope_x, out float slope_y) {
    // special case (normal incidence)
    if (cosTheta > 0.9999) {
        float r = sqrt(U1 / (1 - U1));
        float phi = 6.28318530718 * U2;
        slope_x = r * cos(phi);
        slope_y = r * sin(phi);
        return;
    }

    float sinTheta =
        sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    float tanTheta = sinTheta / cosTheta;
    float a = 1 / tanTheta;
    float G1 = 2 / (1 + sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    float A = 2 * U1 / G1 - 1;
    float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10) tmp = 1e10;
    float B = tanTheta;
    float D = sqrt(
        max(float(B * B * tmp * tmp - (A * A - B * B) * tmp), float(0)));
    float slope_x_1 = B * tmp - D;
    float slope_x_2 = B * tmp + D;
    slope_x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    float S;
    if (U2 > 0.5f) {
        S = 1.f;
        U2 = 2.f * (U2 - .5f);
    } else {
        S = -1.f;
        U2 = 2.f * (.5f - U2);
    }
    float z =
        (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
        (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    slope_y = S * z * sqrt(1.f + slope_x * slope_x);
}
// https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/microfacet.cpp#L238
vec3 TrowbridgeReitzSample(const vec3 wi, const float ax, const float ay, const float U1, const float U2) {
    // 1. stretch wi
    vec3 wiStretched =
        normalize(vec3(ax * wi.x, ay * wi.y, wi.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    float slope_x, slope_y;
    TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2, slope_x, slope_y);

    // 3. rotate
    float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
    slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = ax * slope_x;
    slope_y = ay * slope_y;

    // 5. compute normal
    return normalize(vec3(-slope_x, -slope_y, 1.0));
}
vec3 Sample_wh_SampleVisbleArea(const vec3 wo, float ax, float ay) {
    vec2 u = vec2(rand(), rand());
    bool flip = wo.y < 0;
    vec3 woxzy = vec3(wo.x, wo.z, wo.y);
    vec3 wh = TrowbridgeReitzSample(flip ? -woxzy : woxzy, ax, ay, u[0], u[1]);
    if (flip) {
        wh = -wh;
    }
    vec3 res = vec3(wh.x, wh.z, wh.y);
    return res;
}

// Seems good
vec3 Sample_wh(const vec3 wo, float ax, float ay) {
    // return Sample_wh_SampleVisbleArea(wo, ax, ay);
    vec3 wh;
    vec2 u = vec2(rand(), rand());

    float cosTheta = 0, phi = (2 * PI) * u[1];
    if (ax == ay) {
        float tanTheta2 = ax * ax * u[0] / (1.0f - u[0]);
        cosTheta = 1 / sqrt(1 + tanTheta2);
    } else {
        phi = atan(ay / ax * tan(2 * PI * u[1] + 0.5 * PI));
        if (u[1] > 0.5) {
            phi += PI;
        }
        float sinPhi = sin(phi), cosPhi = cos(phi);
        const float alphax2 = ax * ax, alphay2 = ay * ay;
        const float alpha2 = 1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
        cosTheta = 1 / sqrt(1 + tanTheta2);
    }
    float sinTheta = sqrt(max(0.0, 1.0 - Sqr(cosTheta)));
    wh = SphericalDirection(sinTheta, cosTheta, phi);

    if (!SameHemisphere(wo, wh)) {
        wh = -wh;
    }


    return wh;
}
// MicrofacetDistribution
// https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/microfacet.cpp#L338
float Pdf_TR(const vec3 wo, const vec3 wh) {
    return D(wh) * G1(wo) * abs(dot(wo, wh)) / AbsCosTheta(wo);
}

vec2 SampleUniformDiskPolar() {
    vec2 u = vec2(rand(), rand());
    float r = sqrt(u[0]);
    float theta = 2 * PI * u[1];
    return vec2(r * cos(theta), r * sin(theta));
}

// https://github.com/mmp/pbrt-v4/blob/779d1a78b74aab393853544198189729434121b5/src/pbrt/util/scattering.h#L164
vec3 Sample_wm(vec3 w) {
    // vec3 wh = normalize(vec3(alpha_x * w.x, alpha_y * w.y, w.z));
    vec3 wh = normalize(vec3(alpha_x * w.x, w.z, alpha_y * w.y));
    if (wh.y < 0) {
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
