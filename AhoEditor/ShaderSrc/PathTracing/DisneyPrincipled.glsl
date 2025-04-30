#ifndef GLSL_DISNEY_PRINCIPLED_GLSL
#define GLSL_DISNEY_PRINCIPLED_GLSL

// References
// https://github.com/mitsuba-renderer/mitsuba3/blob/master/src/bsdfs/principled.cpp#L333
// https://schuttejoe.github.io/post/disneybsdf/

// Notation follows mitsuba's impl and is different from pbrt
// wi: incoming direction of the bsdf, view direction V
// wo: outwards direction of the bsdf, light direction L

#include "Random.glsl"
#include "Sampling.glsl"
#include "Disney.glsl"
#include "DisneyPrincipledHelpers.glsl"

vec3 principled_eval(inout State state, const vec3 V, const vec3 L, out float pdf);

vec3 Sample(inout State state, vec3 Vworld, out vec3 Lworld, out float pdf) {
    if (state.cosTheta == 0.0) {
        pdf = 0.0;
        return vec3(0.0);
    }

    Material mat = state.material;
    bool front_side = state.cosTheta > 0.0;
    vec3 Nworld = front_side ? state.N : -state.N;
    state.eta = front_side ? 1.0 / mat.ior : mat.ior;

    // TODO: Not sure about this part
    // Use eta or specular to guide reflect/transmit

    // Store the weights
    float anisotropic = mat.anisotropic;
    float roughness   = mat.roughness;
    float specTrans   = mat.specTrans;
    float metallic    = mat.metallic;
    float clearcoat   = mat.clearcoat;

    bool has_clearcoat = clearcoat > 0.0;
    bool has_spec_trans = specTrans > 0.0;

    // Dielectric reflection weight
    float brdf = (1.0 - metallic) * (1.0 - specTrans);
    // Dielectric refraction weight
    float bsdf = (1.0 - metallic) * specTrans;

    // Local coordinates
    mat3 tbn = ConstructTBN(Nworld);
    vec3 V = normalize(WorldToLocal(Vworld, tbn));

    // Defining main specular reflection distribution
    vec3 H = normalize(SampleGGXVNDF(V, mat.ax, mat.ay, rand(), rand())); // V and H are in the same hemisphere

    // Fresnel coefficient for the main specular
    float F_spec_dielectric = fresnel(abs(dot(V, H)), state.eta);

    // probability definitions
    float prob_spec_reflect = front_side ? (1.0 - bsdf * (1.0 - F_spec_dielectric)) : F_spec_dielectric;
    float prob_spec_trans   = front_side ? bsdf * (1.0 - F_spec_dielectric) : (1.0 - F_spec_dielectric);
    prob_spec_trans = has_spec_trans ? prob_spec_trans : 0.0; 
    float prob_diffuse      = front_side ? brdf : 0;
    // Clearcoat has 1/4 of the main specular reflection energy.
    float prob_clearcoat = float(has_clearcoat) * (front_side ? 0.25 * clearcoat : 0);

    // Normalizing the probabilities
    float rcp_tot_prob = 1.0 / (prob_spec_reflect + prob_spec_trans + prob_clearcoat + prob_diffuse);
    prob_spec_reflect *= rcp_tot_prob;
    prob_spec_trans *= rcp_tot_prob;
    prob_clearcoat *= rcp_tot_prob;
    prob_diffuse *= rcp_tot_prob;

    // Sampling mask definitions
    float sample1 = rand();
    float curr_prob = 0.0;
    float prev[3]; 
    prev[0] = prob_diffuse;
    prev[1] = prev[0] + prob_spec_reflect;
    prev[2] = prev[1] + prob_spec_trans;

    vec3 L;
    // TODO: use remap to reduce extra rand
    if (sample1 < prev[0]) {
        L = SampleCosineHemisphere();
        H = normalize(L + V);
    } else if (sample1 < prev[1]) {
        L = normalize(reflect(-V, H));
    } else if (sample1 < prev[2]) {
        L = normalize(refract(-V, H, state.eta));
    } else { // clearcoat
        L = normalize(reflect(-V, H));
    }

    vec3 f = principled_eval(state, V, L, pdf);
    Lworld = LocalToWorld(L, tbn);

    return f;
}

vec3 principled_eval(inout State state, const vec3 V, const vec3 L, out float pdf) {
    pdf = 0.0;

    Material mat = state.material;
    float roughness   = mat.roughness;
    float specTrans   = mat.specTrans;
    float metallic    = mat.metallic;
    float clearcoat   = mat.clearcoat;
    
    bool front_side = state.cosTheta > 0.0;
    state.eta = front_side ? 1.0 / mat.ior : mat.ior;

    bool is_reflect = V.y * L.y > 0.0;
    bool is_refract = !is_reflect;

    bool has_clearcoat = clearcoat > 0.0;
    bool has_spec_trans = specTrans > 0.0;

    // Reflection weight
    float brdf = (1.0 - metallic) * (1.0 - specTrans);
    // Refraction weight
    float bsdf = (1.0 - metallic) * specTrans;
    
    vec3 H = is_reflect ? normalize(L + V) : normalize(L + V * state.eta);
    if (!front_side) {
        H = -H; // Always pointing outwards of the object
    }

    DotProducts dp;
    SetDotProducts(L, V, H, Y, dp);

    // Fresnel coefficient for the main specular
    float F_spec_dielectric = fresnel(abs(dp.VdotH), state.eta);

    // probability definitions
    float prob_spec_reflect = front_side ? (1.0 - bsdf * (1.0 - F_spec_dielectric)) : F_spec_dielectric;
    float prob_spec_trans   = front_side ? bsdf * (1.0 - F_spec_dielectric) : (1.0 - F_spec_dielectric);
    prob_spec_trans = has_spec_trans ? prob_spec_trans : 0.0;
    float prob_diffuse      = front_side ? brdf : 0;
    // Clearcoat has 1/4 of the main specular reflection energy.
    float prob_clearcoat = float(has_clearcoat) * (front_side ? 0.25 * clearcoat : 0);

    float rcp_tot_prob = 1.0 / (prob_spec_reflect + prob_spec_trans + prob_clearcoat + prob_diffuse);
    prob_spec_reflect *= rcp_tot_prob;
    prob_spec_trans *= rcp_tot_prob;
    prob_clearcoat *= rcp_tot_prob;
    prob_diffuse *= rcp_tot_prob;

    float G1_V = G1(V, mat.ax, mat.ay); // smith_g1
    float G1_L = G1(L, mat.ax, mat.ay);
    float D = D_GGX(H, mat.ax, mat.ay);
    float G = G1_V * G1_L;
    float microfacet_h_pdf = G1_V * D * abs(dp.VdotH) / abs(dp.VdotN);

    float lum = Luminance(mat.baseColor);

    vec3 f = vec3(0.0);

    // Reflection
    if (is_reflect && prob_spec_reflect > 0.0) {
        vec3 F_principled = principled_fresnel(mat.baseColor, lum, bsdf, abs(dp.VdotH), state.eta, metallic, mat.specTint, F_spec_dielectric, front_side);
        f += F_principled * D * G / (4.0 * abs(dp.VdotN) * abs(dp.LdotN)); // ????

        float dwh_dwo_abs = 1.0 / (4.0 * abs(dp.LdotH));
        float reflect_pdf = microfacet_h_pdf * dwh_dwo_abs;
        pdf += prob_spec_reflect * reflect_pdf;
    }
    
    // Transmission
    if (is_refract && prob_spec_trans > 0.0) {
        float denom = abs(dp.VdotH) + state.eta * abs(dp.LdotH);
        f += sqrt(mat.baseColor) * bsdf 
                                * abs((1.0 - F_spec_dielectric) * D * G * state.eta
                                * state.eta * dp.VdotH * dp.LdotH / (dp.VdotN * denom * denom)); 
        float dwh_dwo_abs = (Sqr(state.eta) * abs(dp.LdotH)) / Sqr(abs(dp.VdotH) + state.eta * abs(dp.LdotH));

        float refract_pdf = microfacet_h_pdf * dwh_dwo_abs;
        pdf += prob_spec_trans * refract_pdf;
    }

    // Diffuse, retro reflection, fake subsurface and sheen
    if (is_reflect && brdf > 0.0) {
        float Fi = SchlickWeight(abs(dp.VdotN));
        float Fo = SchlickWeight(abs(dp.LdotN));
        
        // Diffuse
        float f_diff = (1.0 - 0.5 * Fi) * (1.0 - 0.5 * Fo);
        float Rr = 2.0 * roughness * dp.LdotH * dp.LdotH;

        // Retro reflection
        float f_retro = Rr * (Fo + Fi + Fo * Fi * (Rr - 1.0f));
        
        // fake subsurface
        float Fss90 = Rr / 2.0;
        float Fss = mix(1.0, Fss90, Fo) * mix(1.0, Fss90, Fi);
        float f_ss = 1.25 * (Fss * (1.0 / (abs(dp.VdotN) + abs(dp.LdotN)) - 0.5f) + 0.5f);

        f += brdf * mat.baseColor * InvPI * mix(f_diff + f_retro, f_ss, mat.subsurface);
        
        // Sheen
        float Fd = SchlickWeight(abs(dp.LdotH));
        vec3 c_tint = lum > 0.0 ? mat.baseColor / lum : vec3(1.0);
        vec3 c_sheen = mix(vec3(1.0), c_tint, mat.sheenTint);

        f += brdf * mat.sheen * (1.0 - mat.metallic) * Fd * c_sheen;
        pdf += prob_diffuse * CosineHemispherePDF(L);
    }

    // Clearcoat
    if (is_reflect && has_clearcoat) {

    }

    return f * abs(dp.LdotN);
}

#endif