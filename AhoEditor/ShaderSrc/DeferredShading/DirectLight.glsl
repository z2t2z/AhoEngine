#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "../Common/UniformBufferObjects.glsl"
#include "../Common/Material.glsl"
#include "../DeferredShading/BRDF.glsl"
#include "../DeferredShading/Shadow.glsl"

vec3 EvalDirectionalLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
    vec3 baseColor = mat.baseColor;
    float roughness = mat.roughness;
    float metallic = mat.metallic;
    float VoN = dot(V, N);

    vec3 L_out = vec3(0.0);
	for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirLight[i];
        if (light.intensity == 0.0) {
            break;
        }
        vec3 L = light.direction;
        float LoN = dot(L, N);
        if (LoN < 0.0) {
            continue;
        }
        vec3 H = normalize(L + V);
        float LoH = dot(L, H);
        float VoH = dot(V, H);
        float NoH = dot(N, H);

        vec3  F = F_Schlick(F0, VoH);
        // vec3  F = FresnelSchlickRoughness(VoH, F0, roughness);
        float D = D_GGX(roughness, NoH);
        float G = G_Smith(LoN, VoN, roughness);

        vec3 Ks = F;
        vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallic);

        float denom = max(4.0 * LoN * VoN, 1e-6);
        vec3 brdf = (Kd * baseColor * InvPI) + (Ks * D * G / denom);
        
        vec3 L_light = light.color * light.intensity;
        
        L_out += L_light * brdf * LoN * PCSS(vec4(pos, 1.0f), LoN, light.lightProjView);
    }
    return L_out;
}

vec3 EvalPointLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
    return vec3(0.0);
}

// IBL
uniform bool u_SampleEnvLight = false;
uniform samplerCube u_gCubeMap;
uniform samplerCube u_gIrradiance;
uniform samplerCube u_gPrefilter;
uniform sampler2D u_gLUT;
#define MAX_REFLECTION_LOD 4.0

vec3 EvalEnvLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
	vec3 L_out      = vec3(0.0, 0.0, 0.0);
    vec3 baseColor  = mat.baseColor;
    float roughness = mat.roughness;
    float metallic  = mat.metallic;
    float ao        = mat.ao;
    float VoN       = dot(V, N);
    vec3 L          = reflect(-V, N);

    // vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
    vec3  F = F_Schlick(F0, VoN);
    vec3 Ks = F;
    vec3 Kd = (1.0 - Ks) * (1.0 - metallic);

    vec3 irradiance = texture(u_gIrradiance, N).rgb;
    vec3 diffuse = irradiance * baseColor;

    vec3 preFilter = textureLod(u_gPrefilter, L, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_gLUT, vec2(max(VoN, 0.0), roughness)).rg;
    vec3 specular = preFilter * (F * brdf.x + brdf.y);

    // return specular;

    L_out = (Kd * diffuse + specular);// * ao;
    return L_out;
}


#endif