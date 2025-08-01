#type compute
#version 460

#include "Common.glsl"
#include "../Common/UniformBufferObjects.glsl"

layout(binding = 0, rgba16f) uniform writeonly image2D outputImage;

#define FLT_MAX 3.402823466e+38

// G-Buffers
uniform sampler2D u_gPosition; 
uniform sampler2D u_gNormal;
uniform sampler2D u_gPBR;
uniform sampler2D u_gDepth;
uniform sampler2D u_gLitScene;

uniform int u_MostDetailedMip = 0;
uniform int u_MipLevelTotal;
uniform int u_MaxIterations = 128;
uniform float u_Thickness = 0.088;

// Ignore, only for testing
bool NaiveRayMarching(vec3 beginPos, vec3 rayDir, out vec2 hitUV, int maxIterations = 256, float deltaStep = 0.04, float thickNess = 0.04) { 
    vec3 reflectDir = rayDir;
    for (int i = 0; i < maxIterations; i++) {  
        vec3 nxtPos = beginPos + reflectDir * deltaStep * i; 
        vec3 ndc = ToNDC(nxtPos, u_Projection);
        vec2 uv = ndc.xy * 0.5f + 0.5f;
        if (!ValidUV(uv)) 
            return false;
        float sampleDepth = textureLod(u_gPosition, uv, 0).z;
        if (i > 0 && sampleDepth > nxtPos.z && sampleDepth < nxtPos.z + thickNess) {
            hitUV = uv;
            return true;
        } 
    }
    return false;
}


void InitAdvanceRay(vec3 ss_ray_origin, vec3 ss_ray_dir, vec3 ss_ray_dir_inv, vec2 curr_mip_resolution, 
        vec2 curr_mip_resolution_inv, vec2 uv_offset, vec2 floor_offset, out vec3 ss_pos, out float curr_t) {
    vec2 curr_mip_pos = curr_mip_resolution * ss_ray_origin.xy; // [w,h]

    vec2 xy_plane = floor(curr_mip_pos) + floor_offset; // [w,h]
    xy_plane = xy_plane * curr_mip_resolution_inv + uv_offset; // [0,1]

    // o + d * t == p
    vec2 t = xy_plane * ss_ray_dir_inv.xy - ss_ray_origin.xy * ss_ray_dir_inv.xy;
    curr_t = min(t.x, t.y);
    ss_pos = ss_ray_origin + curr_t * ss_ray_dir;
}
bool AdvanceRay(vec3 ss_ray_origin, vec3 ss_ray_dir, vec3 ss_ray_dir_inv, 
    vec2 curr_mip_pos, vec2 curr_mip_resolution_inv, vec2 floor_offset, vec2 uv_offset, float surface_z,
    inout vec3 ss_pos, inout float curr_t) {
    // Boundary planes
    vec2 xy_plane = floor(curr_mip_pos) + floor_offset; // fucking下取整
    xy_plane = xy_plane * curr_mip_resolution_inv + uv_offset;
    vec3 boundary_planes = vec3(xy_plane, surface_z);
    
    // Intersect ray with the half box that is pointing away from the ray origin.
    // o + d * t = p' => t = (p' - o) / d
    vec3 t = boundary_planes * ss_ray_dir_inv - ss_ray_origin * ss_ray_dir_inv;

    t.z = ss_ray_dir.z > 0 ? t.z : FLT_MAX;

    // Choose nearset intersection with a boundary
    float t_min = min(min(t.x, t.y), t.z);
    bool above_surface = surface_z > ss_pos.z;

    bool skipped_tile = floatBitsToUint(t_min) != floatBitsToUint(t.z) && above_surface; // asuint

    curr_t = above_surface ? t_min : curr_t;

    ss_pos = ss_ray_origin + curr_t * ss_ray_dir;

    return skipped_tile;
}

float ValidateHit(vec3 ss_hit_pos, vec3 vs_hit_pos, vec2 uv, vec3 vs_ray_dir, vec2 screen_size, float thickness) {
    // Check if within [0, 1]
    if (!ValidUV(ss_hit_pos.xy)) {
        return 0.0;
    }

    // Avoid self intersection
    vec2 manhattan_dist = abs(ss_hit_pos.xy - uv);
    vec2 inv_screen_size = 2 / screen_size;
    if (manhattan_dist.x < inv_screen_size.x && manhattan_dist.y < inv_screen_size.y) {
        return 0.0;
    }

    // Avoid hitting the background
    ivec2 coords = ivec2(ss_hit_pos.xy * screen_size / 2.0); // Some scaling, why not read from detailed map?
    float surface_z = texelFetch(u_gDepth, coords, 1).r; 
    if (surface_z == 1.0) {
        return 0.0;
    }

    // Avoid hitting from the back
    vec3 vs_normal = texelFetch(u_gNormal, ivec2(ss_hit_pos.xy * screen_size), 0).xyz;
    if (dot(vs_ray_dir, vs_normal) > 0) {
        return 0.0;
    }

    // Test how close two points is in vs
    vec3 vs_hit_surface = ScreenSpaceToViewSpace(vec3(ss_hit_pos.xy, surface_z), u_ProjectionInv);
    float dist = length(vs_hit_pos - vs_hit_surface);

    float confidence = 1 - smoothstep(0, thickness, dist);
    return confidence;
}

// Reference: https://github.com/GPUOpen-Effects/FidelityFX-SSSR/blob/34dcacd1feefcfab2855b82e76c7d711f2020a75/ffx-sssr/ffx_sssr.h#L86
bool HierarchicalRayMarch(vec3 ss_ray_origin, vec3 ss_ray_dir, bool is_mirror, vec2 screen_size, 
    int most_detailed_mip, int max_mip_level, int max_iterations, inout vec3 ss_hit_pos) {
    const vec3 ss_ray_dir_inv = 1 / ss_ray_dir;

    int curr_mip = most_detailed_mip;
    vec2 curr_mip_resolution = GetMipResolution(screen_size, curr_mip);   
    vec2 curr_mip_resolution_inv = 1 / curr_mip_resolution;

    vec2 uv_offset = 0.005 * exp2(most_detailed_mip) / screen_size;
    uv_offset.x = ss_ray_dir.x < 0 ? -uv_offset.x : uv_offset.x;
    uv_offset.y = ss_ray_dir.y < 0 ? -uv_offset.y : uv_offset.y;

    vec2 floor_offset = vec2(ss_ray_dir.x < 0 ? 0 : 1, ss_ray_dir.y < 0 ? 0 : 1);

    float curr_t;
    InitAdvanceRay(ss_ray_origin, ss_ray_dir, ss_ray_dir_inv, curr_mip_resolution, curr_mip_resolution_inv, uv_offset, floor_offset, ss_hit_pos, curr_t);

    int i = 0;
    while (i < max_iterations && curr_mip >= most_detailed_mip) {
        vec2 curr_mip_pos = curr_mip_resolution * ss_hit_pos.xy; 
        float surface_z = texelFetch(u_gDepth, ivec2(curr_mip_pos), curr_mip).r;
        // 结合AdvanceRay的逻辑：
        // ss_hit_pos是当前光线步进到的位置。现在的目标是，AdvanceRay中想在当前mipLevel下，步进掉当前的tile。
        // 采样此时的深度值surface_z，它表示当前tile的深度最小值（最靠近相机的值）。那么如果可以步进当前这个tile，就必须保证光线穿过时，深度值要比surface_z更小。
        // AdvanceRay中构造了一个boundary_planes，是下一个tile的xy边界及surface_z。用它求解光线的交点。
        // 看着很有道理，但总感觉有点怪怪的，反正不是完全理解吧。如果用下一个tile的surface_z就会出现错误。
        // {
        //     // Boundary planes
        //     vec2 xy_plane = floor(curr_mip_pos) + floor_offset;
        //     surface_z = texelFetch(u_gDepth, ivec2(xy_plane), curr_mip).r;
        // }
        bool skipped_tile = AdvanceRay(ss_ray_origin, ss_ray_dir, ss_ray_dir_inv, curr_mip_pos, curr_mip_resolution_inv, floor_offset, uv_offset, surface_z, ss_hit_pos, curr_t);
        curr_mip += skipped_tile ? 1 : -1;
        curr_mip = min(curr_mip, max_mip_level);
        curr_mip_resolution *= skipped_tile ? 0.5 : 2.0;
        curr_mip_resolution_inv *= skipped_tile ? 2.0 : 0.5;
        i += 1;
    }
    bool valid_hit = i < max_iterations;
    return valid_hit;
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screen_size = textureSize(u_gLitScene, 0);
	if (coords.x >= screen_size.x || coords.y >= screen_size.y)
		return;

    vec2 uv = (coords + 0.5) / vec2(screen_size);
    float d = texelFetch(u_gDepth, coords, 0).r;
    vec4 sceneL = texelFetch(u_gLitScene, coords, 0); 
    if (d == 1.0) {
        imageStore(outputImage, coords, sceneL);
        return;
    }

    float z = d;
    int most_detailed_mip = u_MostDetailedMip;

    //ss: screen space, vs: view space, ws: world space
    vec3 ss_ray_origin = vec3(uv, z);
    vec3 vs_ray_origin = ScreenSpaceToViewSpace(ss_ray_origin, u_ProjectionInv);
    vec3 vs_ray_dir = normalize(vs_ray_origin);

    vec3 vs_normal = normalize(texelFetch(u_gNormal, coords, 0).xyz);
    float roughness = texelFetch(u_gPBR, coords, 0).g;
    vec3 vs_reflected_dir = SampleReflectionVector(-vs_ray_dir, vs_normal, roughness, uv);
    vec3 ss_ray_dir = ProjectVsDirToSsDir(vs_ray_origin, vs_reflected_dir, ss_ray_origin, u_Projection);

    vec3 ss_hit_pos;
    bool valid_hit = HierarchicalRayMarch(ss_ray_origin, ss_ray_dir, false, screen_size, most_detailed_mip, u_MipLevelTotal, u_MaxIterations, ss_hit_pos);
    vec3 vs_hit_pos = ScreenSpaceToViewSpace(ss_hit_pos, u_ProjectionInv);

    float confidence = ValidateHit(ss_hit_pos, vs_hit_pos, uv, vs_hit_pos - vs_ray_origin, screen_size, u_Thickness);

    vec4 L = sceneL;
    if (valid_hit && confidence > 0) {
        vec3 reflectedL = texture(u_gLitScene, ss_hit_pos.xy).rgb;
        L.rgb = mix(sceneL.rgb, reflectedL, confidence);
    }

    imageStore(outputImage, coords, L);
    return;
}