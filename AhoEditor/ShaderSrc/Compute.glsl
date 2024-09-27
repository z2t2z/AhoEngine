#type compute

#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std430, binding = 0) buffer DataBuffer {
    uint data[];
};

uniform int u_Width;
uniform int u_Height;

uniform vec3 u_CamPos;

void main() {
    uint globalID_x = gl_GlobalInvocationID.x;
    uint globalID_y = gl_GlobalInvocationID.y;
    uint id = gl_GlobalInvocationID.x;
    //for (int i = 0; i < 10; i++) {
    //    data[i] = 1;
    //}
    data[id] = 1;
}

//uint32_t ConvertToRGBA(const glm::vec4& color) {
//	uint8_t r = (uint8_t)(color.r * 255.0f);
//	uint8_t g = (uint8_t)(color.g * 255.0f);
//	uint8_t b = (uint8_t)(color.b * 255.0f);
//	uint8_t a = (uint8_t)(color.a * 255.0f);
//
//	uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
//	return result;
//}

vec3 PerpixelShading() {
    return vec3(1.0);
}

