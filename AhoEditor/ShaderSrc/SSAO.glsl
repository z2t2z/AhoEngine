#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out float out_color;
in vec2 v_TexCoords;

layout(std140, binding = 2) uniform SSAOUBO {
	mat4 u_Projection;

	vec4 u_Samples[64];
	vec4 u_Info; // width, height, radius, bias
};

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_Noise;

uniform int u_KernelSize = 64;
const int MAX_SAMPLES = 64;

void main() {
	float width = u_Info.x;
	float height = u_Info.y;
	float radius = u_Info.z;
	float bias = u_Info.w;

	const vec2 noiseScale = vec2(width / 4.0, height / 4.0);
	vec3 fragPos = texture(u_gPosition, v_TexCoords).xyz;
	vec3 N = normalize(texture(u_gNormal, v_TexCoords).xyz);
	vec3 rnd = normalize(texture(u_Noise, v_TexCoords * noiseScale).xyz);
	vec3 T = normalize(rnd - N * dot(rnd, N));
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N); // Note: no transpose here since we are not transforming everything to tangent space but the other way around

	float occlusion = 0.0f;
	for (int i = 0; i < MAX_SAMPLES; i++) {
		vec3 samplei = vec3(u_Samples[i].xyz);
		vec3 pos = TBN * samplei;
		pos = fragPos + pos * radius;

		vec4 samplePos = vec4(pos, 1.0f);
		samplePos = u_Projection * samplePos;
		samplePos.xyz /= samplePos.w;
		samplePos.xyz = samplePos.xyz * 0.5f + 0.5f; // [0, 1]
		float sampleDepth = texture(u_gPosition, samplePos.xy).z;
		if (sampleDepth >= pos.z + bias) { // ?
			float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleDepth));
			occlusion += rangeCheck;
		}
	}
	occlusion = 1.0 - (occlusion / u_KernelSize);
	out_color = occlusion;
}