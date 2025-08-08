#type compute
#version 460

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

layout(binding = 0, rgba16f) uniform image2D outputImage;

uniform sampler2D u_SceneDepth;
uniform sampler2D u_DDGIProbeDepth;
uniform sampler2D u_DDGIProbeScene;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 siz = imageSize(outputImage);
    if (coords.x >= siz.x || coords.y >= siz.y)
        return;

    float dScene = texelFetch(u_SceneDepth, coords, 0).r;
    float dProbe = texelFetch(u_DDGIProbeDepth, coords, 0).r;

    if (dProbe < dScene) {
        vec3 color = texelFetch(u_DDGIProbeScene, coords, 0).rgb;
        imageStore(outputImage, coords, vec4(color, 1));
    }
}