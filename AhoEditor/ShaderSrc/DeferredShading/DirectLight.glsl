#ifndef DIRECT_LIGHT_GLSL
#define DIRECT_LIGHT_GLSL

#include "../Common/UniformBufferObjects.glsl"
#include "../Common/Material.glsl"
#include "../DeferredShading/BRDF.glsl"
#include "../DeferredShading/Shadow.glsl"
#include "../PathTracing/IntersectionTest.glsl"

// ---- Directional Light ----
vec3 EvalDirectionalLight(const Material mat, vec3 pos, vec3 F0, vec3 V, vec3 N) {
    vec3 baseColor = mat.baseColor;
    float roughness = mat.roughness;
    float metallic = mat.metallic;
    float NoV = max(0.0001, dot(V, N));
    vec3 L_out = vec3(0.0);
	for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirectionalLight[i];
        if (light.intensity == 0.0) {
            break;
        }
        vec3 L = light.direction; //Pointing towards the light source

#ifdef FEATURE_RAY_TRACE_SHADOW
        Ray shadowRay;
        shadowRay.origin = pos + 0.01 * N; // Avoid self-occluded
        shadowRay.direction = L;
        if (IsOccluded(shadowRay, FLT_MAX))
            continue;       
#endif

        float LoN = dot(L, N);
        if (LoN < 0.0) {
            continue;
        }
        vec3 H = normalize(L + V);
        float VoH = max(0.0001, dot(V, H));
        float NoH = max(0.0001, dot(N, H));

        vec3  F = FresnelSchlick(F0, NoV); // Ks
        float D = D_GGX(roughness, NoH);
        float G = G_Smith(LoN, NoV, roughness);

        vec3 Ks = F;
        vec3 Kd = (1.0 - metallic) * (1.0 - F);
        float denom = max(4.0 * LoN * NoV, 1e-6);
        vec3 brdf = (Kd * baseColor * InvPI) + (Ks * D * G / denom);
        
        vec3 L_light = light.color * light.intensity;
        
        L_out += L_light * brdf * LoN; // * PCSS(vec4(pos, 1.0f), LoN, light.lightProjView);
    }
    return L_out;
}

vec3 EvaluateDirectionalLight(Material material, vec3 pos, vec3 N, vec3 V) {
    vec3 Lsum = vec3(0.0);
    for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
        const DirectionalLight light = u_DirectionalLight[i];
        if (light.intensity == 0.0) {
            continue;
        }
        vec3 L = normalize(light.direction);

#ifdef FEATURE_RAY_TRACE_SHADOW
        Ray shadowRay;
        shadowRay.origin = pos + 0.01 * N; // Avoid self-occluded
        shadowRay.direction = L;
        if (IsOccluded(shadowRay, FLT_MAX))
            continue;       
#endif

        vec3 H = normalize(L + V);
        float NoL = max(dot(N, L), 0.0);
        float NoV = max(dot(N, V), 0.0);
        float NoH = max(dot(N, H), 0.0);
        float VoH = max(dot(V, H), 0.0);

        float alpha = material.roughness * material.roughness;
        float alpha2 = alpha * alpha;

        // Fresnel
        vec3 F0 = mix(vec3(0.04), material.baseColor, material.metallic);
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - VoH, 5.0);

        // NDF
        float denom = (NoH * NoH) * (alpha2 - 1.0) + 1.0;
        float D = alpha2 / (PI * denom * denom);

        // Geometry
        float k = (material.roughness + 1.0);
        k = (k * k) / 8.0;
        float G_V = NoV / (NoV * (1.0 - k) + k);
        float G_L = NoL / (NoL * (1.0 - k) + k);
        float G = G_V * G_L;

        vec3 specular = (D * G * F) / max(4.0 * NoV * NoL, 0.001);
        vec3 kd = (1.0 - F) * (1.0 - material.metallic);
        vec3 diffuse = kd * material.baseColor * InvPI;
        Lsum += (diffuse + specular) * light.color * light.intensity * NoL;
    }
    return Lsum;
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
    vec4 ppworldDir = viewInv * projInv * vec4(clipSpace, 1.0);
    vec3 worldDir = normalize(vec3(ppworldDir.x, ppworldDir.y, ppworldDir.z) / ppworldDir.w);

// #ifdef FEATURE_RAY_TRACE_SHADOW
//     Ray shadowRay;
//     shadowRay.origin = viewPos + 0.01 * worldDir; // Avoid self-occluded
//     shadowRay.direction = worldDir;
//     if (IsOccluded(shadowRay, FLT_MAX))
//         return vec4(0.0, 0.0, 0.0, 1.0); // Shadowed
// #endif

#ifdef FEATURE_ENABLE_IBL				
    vec3 cubemap = texture(u_gCubeMap, worldDir).rgb;
    return vec4(cubemap, 1.0f);
#endif

#ifdef FEATURE_ENABLE_SKYATMOSPHERIC
    const float Rground = 6360.0; 
    vec3 worldPos = viewPos / 1000.0;
    worldPos.y = max(0.01, worldPos.y) + Rground;
    vec2 sampleUV;
    vec3 sunDir = u_SunDir;
    SampleSkyViewLut(worldPos, worldDir, sunDir, sampleUV);
    vec3 lum = 10 * texture(u_SkyviewLUT, sampleUV).rgb;
    lum += GetSunLuminance(worldPos, worldDir, sunDir, Rground);
    return vec4(lum, 1.0);
#endif
    return vec4(0.0, 0.0, 0.0, 1.0);
}


#endif