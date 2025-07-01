#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "Common.glsl"
#include "DDGI_Helpers.glsl"
#include "../PathTracing/IntersectionTest.glsl"
#include "../PathTracing/DataStructs.glsl"

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
    uint globalIdx = gl_GlobalInvocationID.x;
    uint totalProbes = u_DDGIVolumeDesc.probeCount;
    uint raysPerProbe = u_DDGIVolumeDesc.raysPerProbe;
    if (globalIdx >= totalProbes * raysPerProbe) 
        return;

    uint probeIndex = globalIdx / raysPerProbe;
    uint rayIndex   = globalIdx % raysPerProbe;

    // --- To 3D grid coords ---
    uint probeCountX = u_DDGIVolumeDesc.probeCountX;
    uint probeCountY = u_DDGIVolumeDesc.probeCountY;
    uint probeCountZ = u_DDGIVolumeDesc.probeCountZ;
    uvec3 probeGridCoord;
    probeGridCoord.z = probeIndex / (probeCountX * probeCountY);
    uint rem = probeIndex % (probeCountX * probeCountY);
    probeGridCoord.y = rem / probeCountX;
    probeGridCoord.x = rem % probeCountX;

    // --- To world space ---
    vec3 probeWorldPos = GetProbeWorldPosition(probeGridCoord, u_DDGIVolumeDesc);
    vec3 probeRayDir = GetProbeRayDirection(rayIndex, u_DDGIVolumeDesc);

    Ray ray;
    ray.origin = probeWorldPos;
    ray.direction = probeRayDir;

    // --- Trace the probe ray ---
    HitInfo info = InitHitInfo();
    RayTrace(ray, info);
    
    // --- Miss ray, store miss radiance, early exit ---
    if (!info.hit) {
        // TODO: What is miss radiance?
        // DDGIStoreProbeRayMiss(RayData, outputCoords, volume, GetGlobalConst(app, skyRadiance));
        return;
    }

    // --- Hit ray ---
    
}