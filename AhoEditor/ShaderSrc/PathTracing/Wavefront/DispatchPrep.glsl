#type compute
#version 460
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "../SSBO.glsl"

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    s_DispatchParams[0].numx = (s_QueueCounter + 255u) / 256u;
    s_DispatchParams[0].numy = 1;
    s_DispatchParams[0].numz = 1;
}
