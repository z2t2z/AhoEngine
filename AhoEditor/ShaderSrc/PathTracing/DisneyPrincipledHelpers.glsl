#ifndef DISNEY_PRINCIPLED_HELPERS_GLSL
#define DISNEY_PRINCIPLED_HELPERS_GLSL

#include "Math.glsl"
#include "Sampling.glsl"

void CalDistParams(float anisotropic, float roughness, out float ax, out float ay) {
    float roughness2 = roughness * roughness;
    if (anisotropic == 0) {
        ax = max(0.001, roughness2);
        ay = ax;
        return;
    }
    float aspect = sqrt(1.0 - 0.9 * anisotropic);
    ax = max(0.001, roughness2 / aspect);
    ay = max(0.001, roughness2 * aspect);
}

Material InitMaterial() {
    Material mat;
    mat.ior = 1.5; // glass
    mat.baseColor = vec3(1.0);
    mat.roughness = 0.5;
    mat.sheenTint = 0.1;
    mat.metallic = 0.1;
    mat.subsurface = 0.0;
    mat.specular = 0.0;
    mat.specTrans = 0.0;
    mat.specTint = 0.0;
    mat.anisotropic = 0.0;
    mat.sheen = 0.0;
    mat.clearcoat = 0.0;
    mat.clearcoatGloss = 0.0;
    CalDistParams(mat.anisotropic, mat.roughness, mat.ax, mat.ay);
    return mat;
}

// Evaluation of diffuse, retro reflection, fake subsurface and sheen
vec3 eval_diffuse(const State state, const DotProducts dp, const vec3 V, const vec3 L, const vec3 H, out float pdf) {
    const Material mat = state.material;
    float brdf = (1.0f - mat.metallic) * (1.0f - mat.specTrans);
    pdf = CosineHemispherePDF(L);

    vec3 f = vec3(0.0);

    float Fi = SchlickWeight(abs(dp.VdotN));
    float Fo = SchlickWeight(abs(dp.LdotN));
    
    // Diffuse
    float f_diff = (1.0 - 0.5 * Fi) * (1.0 - 0.5 * Fo);
    float Rr = 2.0 * mat.roughness * dp.LdotH * dp.LdotH;

    // Retro reflection
    float f_retro = Rr * (Fo + Fi + Fo * Fi * (Rr - 1.0f));
    
    // fake subsurface
    float Fss90 = Rr / 2.0;
    float Fss = mix(1.0, Fss90, Fo) * mix(1.0, Fss90, Fi);
    float f_ss = 1.25 * (Fss * (1.0 / (abs(dp.VdotN) + abs(dp.LdotN)) - 0.5f) + 0.5f);

    f += brdf * mat.baseColor * InvPI * mix(f_diff + f_retro, f_ss, mat.subsurface); // * abs(dp.LdotN);
    
    // Sheen
    float Fd = SchlickWeight(abs(dp.LdotH));
    float lum = Luminance(mat.baseColor);
    vec3 c_tint = lum > 0.0 ? mat.baseColor / lum : vec3(1.0);
    vec3 c_sheen = mix(vec3(1.0), c_tint, mat.sheenTint);

    f += mat.sheen * (1.0 - mat.metallic) * Fd * c_sheen; // * abs(dp.LdotN);

    return f;
}

// cos_theta_i: cosine of the angle between the incoming light direction and the normal
// R0: reflectance at normal incidence (F0)
// eta: refractive index (eta = n1/n2)
// float calc_schlick(float R0, float cos_theta_i, float eta) {
//     float w = SchlickWeight(abs(cos_theta_i));
//     return mix(w, 1.0, R0);
// }
float schlick_weight(float cos_i) {
    float m = clamp(1.0 - cos_i, 0.0, 1.0);
    float m2 = m * m;
    // m^5
    return m2 * m2 * m;
}
vec3 calc_schlick(vec3 R0, float cos_theta_i, float eta) {
    bool outside = cos_theta_i >= 0.0;
    float rcpEta = 1.0 / eta;
    float eta_it = outside ? eta    : rcpEta;
    float eta_ti = outside ? rcpEta : eta;

    float cosT2 = 1.0 - (1.0 - cos_theta_i * cos_theta_i) * (eta_ti * eta_ti);
    float cosT = sqrt(max(cosT2, 0.0));

    float w = (eta_it > 1.0)
              ? schlick_weight(abs(cos_theta_i))
              : schlick_weight(cosT);

    // return mix(vec3(w), vec3(1.0), R0);
    return mix(R0, vec3(1.0), vec3(w)); 
}
float schlick_R0_eta(float eta) {
    float v = (eta - 1.0) / (eta + 1.0);
    return v * v;
}
vec3 principled_fresnel(vec3 baseColor, float lum, float bsdf, float cos_theta_i, float eta, float metallic, float specTint, float F_dielectric, bool front_side) {
    bool outside = cos_theta_i >= 0.0;
    float rcp_eta = 1.0 / eta;
    float eta_it = outside ? eta : rcp_eta;
    vec3 F_schlick = vec3(0.0);
    
    // Metallic component based on Schlick
    F_schlick += metallic * calc_schlick(baseColor, cos_theta_i, eta);

    // Tinted dielectric component based on Schlick.
    if (specTint > 0.0) {
        vec3 c_tint = lum > 0.0 ? baseColor / lum : vec3(1.0);
        vec3 F0_spec_tint = c_tint * schlick_R0_eta(eta_it);
        F_schlick += (1.0 - metallic) * specTint * calc_schlick(F0_spec_tint, cos_theta_i, eta);
    }

    vec3 F_front = (1.0 - metallic) * vec3(1.0 - specTint) * F_dielectric + F_schlick;
    return front_side ? F_front : vec3(bsdf * F_dielectric);
}


#endif