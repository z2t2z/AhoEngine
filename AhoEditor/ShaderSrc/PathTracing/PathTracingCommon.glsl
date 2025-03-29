#ifndef PT_COMMON_GLSL
#define PT_COMMON_GLSL

#include "DataStructs.glsl"
#include "../UniformBufferObjects.glsl"
#include "./Random.glsl"

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

#endif