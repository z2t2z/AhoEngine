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
layout(location = 0) out vec4 out_color;

in vec2 v_TexCoords;
layout(std140, binding = 1) uniform GeneralUBO{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view

	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
	ivec4 info;
};

uniform float u_Linear;
uniform float u_Quadratic;

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gSSAO;

void main() {
	vec3 fragPos = texture(u_gPosition, v_TexCoords).rgb;
	vec3 normal = texture(u_gNormal, v_TexCoords).rgb;
	vec3 albedo = texture(u_gAlbedo, v_TexCoords).rgb;
	vec3 lightColor = u_LightColor[0].rgb;
	vec3 lightPos = (u_View * u_LightPosition[0]).xyz;
	float AO = texture(u_gSSAO, v_TexCoords).r;

	vec3 ambient = vec3(0.3f * AO * albedo);
	vec3 viewDir = normalize(-fragPos);

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * albedo * lightColor;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specFac = pow(max(dot(normal, halfwayDir), 0.0f), 16.0f);
	vec3 spec = lightColor * specFac;

	float dis = length(lightPos - fragPos);
	float attenuation = 1.0f / (dis * dis);
	vec3 result = ambient + (diffuse + spec);

	out_color = vec4(result, 1.0f);
}