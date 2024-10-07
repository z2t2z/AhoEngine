#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoords;

// Light info
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform float u_Linear;
uniform float u_Quadratic;

uniform sampler2D g_PositionDepth;
uniform sampler2D g_Normal;
uniform sampler2D g_Albedo;
uniform sampler2D g_SSAO;

void main() {
	vec3 position = texture(g_PositionDepth, v_TexCoords).rgb;
	vec3 normal = texture(g_Normal, v_TexCoords).rgb;
	vec3 albedo = texture(g_Albedo, v_TexCoords).rgb;
	float AO = texture(g_SSAO, v_TexCoords).r;

	vec3 ambient = vec3(0.3f * AO);
	vec3 viewDir = normalize(-position);

	vec3 lightDir = normalize(u_LightPosition - position);
	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * albedo * u_LightColor;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specFac = pow(max(dot(normal, halfwayDir), 0.0f), 16.0f);
	vec3 spec = u_LightColor * specFac;

	float dis = length(u_LightPosition - position);
	float attenuation = 1.0f / (1.0f + u_Linear * dis + u_Quadratic * dis * dis);
	vec3 result = ambient + (diffuse + spec) * attenuation;

	color = vec4(result, 1.0f);
}