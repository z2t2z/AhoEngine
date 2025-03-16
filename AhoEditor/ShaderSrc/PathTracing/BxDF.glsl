#ifndef BXDF_GLSL
#define BXDF_GLSL

#include "./SSBO.glsl"
#include "./DataStructs.glsl"
#include "./Sampling.glsl"


vec3 f(const vec3 wo, const vec3 wi) {
    return InvPI * vec3(1.0, 1.0, 1.0); // TODO
}

bool SameHemisphere(const vec3 w, const vec3 wp) {
    return w.z * wp.z > 0;
}

float AbsCosTheta(const vec3 w) { 
    return abs(w.z); 
}

float Pdf(const vec3 wo, const vec3 wi) {
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPI : 0;
}

// Lambertian BRDFs
vec3 Sample_f(vec3 wo, out vec3 wi, out float pdf) {
    wi = SampleCosineHemisphere();
    if (wo.z < 0) {
        wi.z *= -1;
    }
    pdf = Pdf(wo, wi);
    return f(wo, wi);
}

vec3 LocalToWorld(in out vec3 wi_local, vec3 normal) {
    vec3 t = (abs(normal.y) < 0.999f) ? normalize(cross(Y, normal)) : normalize(cross(X, normal));
    vec3 b = normalize(cross(normal, t));
    return vec3(
        wi_local.x * t.x + wi_local.y * normal.x + wi_local.z * b.x,
        wi_local.x * t.y + wi_local.y * normal.y + wi_local.z * b.y,
        wi_local.x * t.z + wi_local.y * normal.z + wi_local.z * b.z
    );
}

#endif
