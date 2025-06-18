#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "../Common/UniformBufferObjects.glsl"
#include "../Common/Material.glsl"
#include "../DeferredShading/BRDF.glsl"
#include "../DeferredShading/Shadow.glsl"

// ---- Directional Light ----
vec3 EvalDirectionalLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
    vec3 baseColor = mat.baseColor;
    float roughness = mat.roughness;
    float metallic = mat.metallic;
    float VoN = max(0.0001, dot(V, N));
    vec3 L_out = vec3(0.0);
	for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirectionalLight[i];
        if (light.intensity == 0.0) {
            break;
        }
        vec3 L = light.direction; // Note facing direction
        float LoN = dot(L, N);
        if (LoN < 0.0) {
            continue;
        }
        vec3 H = normalize(L + V);
        float VoH = max(0.0001, dot(V, H));
        float NoH = max(0.0001, dot(N, H));

        vec3  F = F_Schlick(F0, VoH);
        float D = D_GGX(roughness, NoH);
        float G = G_Smith(LoN, VoN, roughness);

        vec3 Ks = F;
        vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallic);

        float denom = max(4.0 * LoN * VoN, 1e-6);
        vec3 brdf = (Kd * baseColor * InvPI) + (Ks * D * G / denom);
        
        vec3 L_light = light.color * light.intensity;
        
        L_out += L_light * brdf * LoN; // * PCSS(vec4(pos, 1.0f), LoN, light.lightProjView);
    }
    return L_out;
}

// ---- Point Light ----
vec3 EvalPointLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
    return vec3(0.0);
}


// ---- Environment Light ----
// IBL
uniform float u_PrefilterMaxMipLevel;
uniform samplerCube u_gCubeMap;
uniform samplerCube u_gIrradiance;
uniform samplerCube u_gPrefilter;
uniform sampler2D u_gBRDFLUT;
vec3 EvalEnvLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
	vec3 L_out      = vec3(0.0);
    vec3 baseColor  = mat.baseColor;
    float roughness = mat.roughness;
    float metallic  = mat.metallic;
    float ao        = mat.ao;
    float NoV = max(dot(N, V), 0.0001f);

    vec3 F = FresnelSchlick(F0, NoV); // Ks

    vec3 irradiance = textureLod(u_gIrradiance, N, 0).rgb;
    vec3 Kd = (1.0 - metallic) * (1.0 - F);
    vec3 diffuse = irradiance * baseColor;

    vec3 R = 2.0 * NoV * N - V; 
    float mipLevel = clamp(roughness * u_PrefilterMaxMipLevel, 0.0, u_PrefilterMaxMipLevel);
    vec3 prefilteredColor = textureLod(u_gPrefilter, R, mipLevel).rgb;
    vec2 brdf = textureLod(u_gBRDFLUT, vec2(NoV, roughness), 0).rg;
    vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);
    L_out = Kd * diffuse + specular;
    return L_out;
}
// ---- Evaluate infinite background ----
// Atmospheric 
#ifdef FEATURE_ENABLE_SKYATMOSPHERIC
    uniform sampler2D u_SkyviewLUT;
#endif
vec4 EvalBackground(vec2 uv, vec3 viewPos, mat4 viewInv, mat4 projInv) {
    vec3 clipSpace = vec3(uv * 2.0 - vec2(1.0), 1.0);
    // vec4 ppworldDir = u_ViewInv * u_ProjectionInv * vec4(clipSpace, 1.0);
    vec4 ppworldDir = viewInv * projInv * vec4(clipSpace, 1.0);
    vec3 worldDir = normalize(vec3(ppworldDir.x, ppworldDir.y, ppworldDir.z) / ppworldDir.w);

#ifdef FEATURE_ENABLE_IBL				
    vec3 cubemap = texture(u_gCubeMap, worldDir).rgb;
    cubemap = pow(cubemap, vec3(1.0 / 2.2f)); // gamma correction
    return vec4(cubemap, 1.0f);
#endif

#ifdef FEATURE_ENABLE_SKYATMOSPHERIC
    const float Rground = 6360.0; 
    vec3 worldPos = viewPos / 1000.0;
    worldPos.y = max(0.01, worldPos.y) + Rground;
    vec2 sampleUV;
    vec3 sunDir = u_SunDir;
    SampleSkyViewLut(worldPos, worldDir, sunDir, sampleUV);
    vec3 lum = 50 * texture(u_SkyviewLUT, sampleUV).rgb;
    lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0)) * 2.0 + 0.15);
    lum = jodieReinhardTonemap(lum);
    lum = pow(lum, vec3(1.0 / 2.2)) + GetSunLuminance(worldPos, worldDir, sunDir, Rground);
    return vec4(lum, 1.0);
#endif
    return vec4(0.0, 0.0, 0.0, 1.0);
}


#endif