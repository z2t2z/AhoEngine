#ifndef MIRCROFACET_REFLECTION_GLSL 
#define MIRCROFACET_REFLECTION_GLSL 

#include "./Math.glsl"
#include "./BxDFCommon.glsl"

vec3 f_MR(vec3 wo, vec3 wi) {
    float cosThetaO = AbsCosTheta(wo);
    float cosThetaI = AbsCosTheta(wi);
    vec3 wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0 || cosThetaO == 0) {
        return vec3(0.0, 0.0, 0.0);
    }
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) {
        return vec3(0.0, 0.0, 0.0);
    }
    wh = normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    // float F = FrDielectric(dot(wi, Faceforward(wh, vec3(0, 1, 0))));
    float F = FrDielectric(dot(wi, wh)); 

    const vec3 R = vec3(1.0, 1.0, 1.0);
    return R * D(wh) * G(wo, wi) * F / (4 * cosThetaI * cosThetaO);
}


vec3 Sample_f_MR(vec3 wo, out vec3 wi, out float pdf) {
    if (wo.y == 0) {
        return vec3(0.0, 0.0, 0.0);
    }
    vec3 wh = Sample_wh(wo);
    if (dot(wo, wh) < 0) {
        return vec3(0.0, 0.0, 0.0);
    }   
    wi = reflect(-wo, wh);
    if (!SameHemisphere(wo, wi)) {
        return vec3(0.0, 0.0, 0.0);
    }

    // Compute PDF of wi for microfacet reflection
    pdf = Pdf_TR(wo, wh) / (4 * dot(wo, wh));
    return f_MR(wo, wi);    
}

#endif