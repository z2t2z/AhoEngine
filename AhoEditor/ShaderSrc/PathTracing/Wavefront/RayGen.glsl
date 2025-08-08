#type compute
#version 460
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define PATH_TRACING

#include "../SSBO.glsl"
#include "../PathTracingCommon.glsl"

uniform int u_WriteIndex;
uniform int u_SrcWidth;
uniform int u_SrcHeight;

#define FLT_MAX 3.402823466e+38

Payload InitPayload(const Ray ray, int pixelIndex);

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= u_SrcWidth || coords.y >= u_SrcHeight)
		return;

	int pixelIndex = coords.y * u_SrcWidth + coords.x;
	Ray ray = GetRayFromScreenSpace(coords, vec2(u_SrcWidth, u_SrcHeight));
	Payload payload = InitPayload(ray, pixelIndex);

	if (u_WriteIndex == 0) {
		s_Payload0[pixelIndex] = payload;
	}
	else {
		s_Payload1[pixelIndex] = payload;
	}
}

Payload InitPayload(const Ray ray, int pixelIndex) {
	Payload payload;
	payload.alive = true;
	payload.origin = ray.origin;
	payload.direction = ray.direction;
	payload.throughput = vec3(1);
	payload.radiance = vec3(0);
	payload.pdf = 1;
	payload.bounce = 1;
	payload.pixelIndex = pixelIndex;
	payload.eta = 1.5;
	payload.N = vec3(0, 1, 0);
	payload.pos = vec3(0);
	payload.cosTheta = 1.0;
	return payload;
}