#type compute

#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer DataBuffer {
    float data[];
};

void main() {
    uint id = gl_GlobalInvocationID.x;
    data[id] = data[id] * 2.0;  // 将每个数据项乘以2
}
