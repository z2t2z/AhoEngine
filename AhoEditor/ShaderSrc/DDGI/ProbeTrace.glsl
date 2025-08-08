#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "DDGICommon.glsl"
#include "DDGIHelpers.glsl"
#include "Lighting.glsl"
#include "../DeferredShading/BRDF.glsl"
#include "../PathTracing/PathTracingCommon.glsl"
#include "../PathTracing/IntersectionTest.glsl"
#include "../PathTracing/DataStructs.glsl"

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;

layout(binding = 0, rgba16f) uniform image2D ProbeRadianceDistance;

void Miss(inout DDGIPayload payload, const Ray ray, const HitInfo info);
void CloestHit(inout DDGIPayload payload, const Ray ray, const HitInfo info, const DDGIVolumeDescGPU volume);

layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
void main() {
    uint volumeIndex = 0; // GetDDGIVolumeIndex(); //TODO;
    DDGIVolumeDescGPU volume = u_DDGIVolumeDesc;

    // --- Ray, probe index in this thread ---
    int rayIndex = int(gl_WorkGroupID.x);
    int probeIndex = int(gl_WorkGroupID.y);

    // int rayIndex = int(gl_GlobalInvocationID.x);
    // int probeIndex = int(gl_GlobalInvocationID.y);    

    // --- To 3d grid coords --- 
    ivec3 probeCoords = GetProbeCoords(probeIndex, volume);

    // --- Get world position, ray direction ---
    vec3 probeWorldPosition = DDGIGetProbeWorldPosition(probeCoords, volume);
    vec3 rayDirection = DDGIGetProbeRayDirection(rayIndex, volume);

    // --- Ray Trace ---
    Ray ray;
    ray.origin = probeWorldPosition;
    ray.direction = rayDirection;
    HitInfo info = InitHitInfo();
    RayTrace(ray, info);
    
    DDGIPayload payload;
    payload.radiance = vec3(0);
    payload.t = 1e8; // Default far distance
    if (!info.hit) {
        Miss(payload, ray, info);
    } else {
        CloestHit(payload, ray, info, volume);
    }

    imageStore(ProbeRadianceDistance, ivec2(rayIndex, probeIndex), vec4(payload.radiance, payload.t));
}

// payload.t = -1.0; it??
void Miss(inout DDGIPayload payload, const Ray ray, const HitInfo info) {
    // payload.radiance = vec3(0.01);
    payload.t = -2;
}

void CloestHit(inout DDGIPayload payload, const Ray ray, const HitInfo info, const DDGIVolumeDescGPU volume) {
    PrimitiveDesc p = s_bPrimitives[info.globalPrimtiveId];
    State state;
    RetrievePrimInfo(state, p, info.uv); // Get material info
    Material mat = state.material;
    vec3 normal = state.N;

    vec3 N = normalize(normal);
    vec2 uv = info.uv;
    float u = uv.x, v = uv.y, w = 1.0 - u - v;
    vec3 pos = ray.origin + info.t * ray.direction;
    // vec3 pos = w * p.v[0].position + u * p.v[1].position + v * p.v[2].position;  // Don't use ray.origin + info.t * ray.direction

    bool front_face = bool(dot(ray.direction, N) < 0.0);
    if (!front_face) {
        payload.t = -2;
        N = -N; // Flip normal
        return;
    }
    
    vec3 radiance = EvaluateDirectDiffuse(mat, pos, N, ray.direction);
    radiance += DiffuseBRDF(mat.baseColor) * SampleDDGIIrradiance(u_DDGIVolumeDesc, pos, N, -ray.direction);
    
    payload.radiance = radiance;
    payload.t = info.t;
}