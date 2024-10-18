#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

out float color;

in vec2 v_TexCoords;

uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform float u_Linear;
uniform float u_Quadratic;

uniform sampler2D g_PositionDepth;
uniform sampler2D g_Normal;
uniform sampler2D u_NoiseTexture;

uniform float u_Width;
uniform float u_Height;

uniform float u_Radius;
uniform int u_KernelSiz;

int MAX_SAMPLES_SIZE = 64;
uniform vec3 u_Samples[64];

uniform mat4 u_Projection;

void main() {
	vec3 normal = texture(g_Normal, v_TexCoords).rgb;
	vec3 position = texture(g_PositionDepth, v_TexCoords).rgb;
	vec2 scalingFactor = vec2(u_Width / 4.0f, u_Height / 4.0f);
	vec3 rndVec = texture(u_NoiseTexture, v_TexCoords * scalingFactor).xyz;

	vec3 tangent = normalize(rndVec - normal * dot(rndVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float AO = 0.0f;
	int MAX_I = min(MAX_SAMPLES_SIZE, u_KernelSiz);
	for (int i = 0; i < MAX_I; i++) {
		vec3 samplePos = TBN * u_Samples[i]; // to view space
		samplePos = position + samplePos * u_Radius;
	
		vec4 offset = vec4(samplePos, 1.0f);
		offset = u_Projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float depth = -texture(g_PositionDepth, offset.xy).w;
		float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(position.z - depth));
		AO += (depth >= samplePos.z ? 1.0 : 0.0) * rangeCheck;
	}
	AO = 1.0 - (AO / u_KernelSiz);
	color = AO;
}