#ifndef PT_COMMON_GLSL
#define PT_COMMON_GLSL

#ifndef PATH_TRACING
#define PATH_TRACING
#endif

#include "../Common/Material.glsl"
#include "../Common/UniformBufferObjects.glsl"
#include "SSBO.glsl"
#include "DataStructs.glsl"
#include "GlobalVars.glsl"
#include "Random.glsl"

/* 
    TODO: Figure out how UE does this:
    https://github.com/EpicGames/UnrealEngine/blob/585df42eb3a391efd295abd231333df20cddbcf3/Engine/Shaders/Private/RayTracing/RayTracingCommon.ush#L508
*/
Ray GetRayFromScreenSpace(vec2 coords, vec2 resolution) {
	vec2 jitter = vec2(rand(), rand());
	vec2 uv = (coords + jitter) / resolution;

    // vec4 worldPos = vec4(coords / resolution * vec2(2.0f) - vec2(1.0f), 1.0, 1.0);
    vec4 worldPos = vec4(uv * vec2(2.0f) - vec2(1.0f), 1.0, 1.0);

    worldPos = u_ProjectionInv * worldPos;

    worldPos = u_ViewInv * worldPos;
    if (worldPos.w != 0.0f) {
        worldPos /= worldPos.w;
    }

    Ray ray;
    ray.origin = vec3(u_ViewPosition);
    ray.direction = normalize(vec3(worldPos) - vec3(u_ViewPosition));
    return ray;
}

Ray RayCasting(uint x, uint y, vec2 resolution) {
	Ray ray;
	ray.origin = vec3(u_ViewPosition);
	// Image space to screen space
	float ssx = float(x) / resolution.x, ssy = float(y) / resolution.y;
	// Screen space to NDC
	vec2 ndc = vec2(ssx * 2.0f - 1.0f, ssy * 2.0f - 1.0f);
	// NDC to view space
	vec4 vs = u_ProjectionInv * vec4(ndc, 1.0f, 1.0f);

	vec3 rayDirection = vec3(u_ViewInv * vec4(normalize(vec3(vs) / vs.w), 0)); // World space
	ray.direction = normalize(rayDirection);
	//vs.z = -1.0f;
	//vs.w = 0.0f;
	//// View space to world space
	//vec3 ws = vec3(u_ViewInv * vs);
	//ray.Direction = normalize(ws);
	return ray;
}

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

Material GetMaterial(const PrimitiveDesc p, vec2 uv, inout vec3 normal) {
    const TextureHandles handle = s_TextureHandles[p.meshId];
    Material mat;
    mat.baseColor = handle.baseColor;
    mat.metallic = handle.metallic;
    mat.emissive = handle.emissive;
    mat.emissiveScale = handle.emissiveScale;
    mat.subsurface = handle.subsurface;
    mat.specular = handle.specular;
    mat.specTint = handle.specTint;
    mat.specTrans = handle.specTrans;
    mat.ior = handle.ior;
    mat.clearcoat = handle.clearcoat;
    mat.clearcoatGloss = handle.clearcoatGloss;
    mat.anisotropic = handle.anisotropic;
    mat.sheenTint = handle.sheenTint;
    mat.sheen = handle.sheen;
    mat.roughness = handle.roughness;
    mat.ax = handle.ax;
    mat.ay = handle.ay;
    float u = uv.x, v = uv.y, w = 1.0 - u - v;
    vec3 N = normalize(w * p.v[0].normal + u * p.v[1].normal + v * p.v[2].normal);
    vec3 hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;  // Don't use ray.origin + info.t * ray.direction
    vec2 TextureUV = vec2(w * p.v[0].u + u * p.v[1].u + v * p.v[2].u, w * p.v[0].v + u * p.v[1].v + v * p.v[2].v); 
    if (int64_t(handle.albedoHandle) != 0) {
        mat.baseColor = texture(handle.albedoHandle, TextureUV).rgb;
    }
    mat.baseColor = pow(mat.baseColor, vec3(2.2));
    
    if (int64_t(handle.normalHandle) != 0) {
        vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
        T = normalize(T);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T); // Right-handed
        mat3 TBN = mat3(T, B, N);
        vec3 n = texture(handle.normalHandle, TextureUV).xyz * 2.0 - 1.0;
        N = normalize(TBN * n);
    }
    normal = N;

    if (int64_t(handle.roughnessHandle) != 0) {
        mat.roughness = texture(handle.roughnessHandle, TextureUV).r;
        CalDistParams(mat.anisotropic, mat.roughness, mat.ax, mat.ay); 
    }
    if (int64_t(handle.metallicHandle) != 0) {
        mat.metallic = texture(handle.metallicHandle, TextureUV).r;
    }
        
    return mat;
}

void RetrievePrimInfo(out State state, in PrimitiveDesc p, vec2 uv) {
    const TextureHandles handle = s_TextureHandles[p.meshId];
    Material mat;
    mat.baseColor = handle.baseColor;
    mat.metallic = handle.metallic;
    mat.emissive = handle.emissive;
    mat.emissiveScale = handle.emissiveScale;
    mat.subsurface = handle.subsurface;
    mat.specular = handle.specular;
    mat.specTint = handle.specTint;
    mat.specTrans = handle.specTrans;
    mat.ior = handle.ior;
    mat.clearcoat = handle.clearcoat;
    mat.clearcoatGloss = handle.clearcoatGloss;
    mat.anisotropic = handle.anisotropic;
    mat.sheenTint = handle.sheenTint;
    mat.sheen = handle.sheen;
    mat.roughness = handle.roughness;
    mat.ax = handle.ax;
    mat.ay = handle.ay;

    float u = uv.x, v = uv.y, w = 1.0 - u - v;
    vec3 N = normalize(w * p.v[0].normal + u * p.v[1].normal + v * p.v[2].normal);
    vec3 hitPos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;  // Don't use ray.origin + info.t * ray.direction
    state.pos = hitPos;
    vec2 TextureUV = vec2(w * p.v[0].u + u * p.v[1].u + v * p.v[2].u, w * p.v[0].v + u * p.v[1].v + v * p.v[2].v); 
    if (int64_t(handle.albedoHandle) != 0) {
        mat.baseColor = texture(handle.albedoHandle, TextureUV).rgb;
    }
    // mat.baseColor = pow(mat.baseColor, vec3(2.2));
    if (int64_t(handle.normalHandle) != 0) {
        vec3 T = w * p.v[0].tangent + u * p.v[1].tangent + v * p.v[2].tangent;
        T = normalize(T);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T); // Right-handed
        mat3 TBN = mat3(T, B, N);
        vec3 n = texture(handle.normalHandle, TextureUV).xyz * 2.0 - 1.0;
        N = normalize(TBN * n);
    } 
    state.N = N;    
    if (int64_t(handle.roughnessHandle) != 0) {
        mat.roughness = texture(handle.roughnessHandle, TextureUV).r;
        CalDistParams(mat.anisotropic, mat.roughness, mat.ax, mat.ay); 
    }
    if (int64_t(handle.metallicHandle) != 0) {
        mat.metallic = texture(handle.metallicHandle, TextureUV).r;
    }
    state.material = mat;
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


#endif