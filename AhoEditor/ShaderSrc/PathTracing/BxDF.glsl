#ifndef BXDF_GLSL
#define BXDF_GLSL

#include "./SSBO.glsl"
#include "./DataStructs.glsl"
#include "./Random.glsl"

struct BSDFSample {

};

// Lambertian BRDFs
vec3 Sample_f(const vec3 wo, out vec3 wi,
        const Point2f &u, Float *pdf, BxDFType *sampledType) {
       *wi = CosineSampleHemisphere(u);
       if (wo.z < 0) wi->z *= -1;

    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

void f_without_R(const vec3 wo, const vec3 wi) {
    return InvPI;
}

bool SameHemisphere(in out vec3 w, in out vec3 wp) {
    return w.z * wp.z > 0;
}

float AbsCosTheta(const vec3 w) { 
    return abs(w.z); 
}

float Pdf(const vec3 wo, const vec3 wi) const {
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}


#endif
