#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

layout(binding = 0, rgba16f) uniform image2D outputImage;

#include "Common.glsl"

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;
uniform sampler2D u_SceneDepth;
uniform sampler2D u_DDGIProbeDepth;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 siz = imageSize(outputImage);
    if (pixelCoord.x >= siz.x || pixelCoord.y >= siz.y)
		return;
}