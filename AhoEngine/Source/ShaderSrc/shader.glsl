#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

out vec3 v_Position;
out vec3 v_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model; // TODO

void main() {
	v_Position = (u_Model * vec4(a_Position, 1.0)).xyz;
	gl_Position = u_ViewProjection * vec4(v_Position, 1.0);
	v_Normal = a_Normal;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 v_Position;
in vec3 v_Normal;

uniform vec3 u_ViewPosition;
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform vec3 u_Color;

void main() {

	// Blinn-Phong:
	// Ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * u_LightColor;
	// Diffuse
	vec3 norm = normalize(v_Normal);         // 法线归一化
	vec3 lightDir = normalize(u_LightPosition - v_Position); // 计算光源方向
	float diff = max(dot(norm, lightDir), 0.0);    // 漫反射分量
	vec3 diffuse = diff * u_LightColor;
	// Specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(u_ViewPosition - v_Position);  // 视线方向
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong中的半程向量
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32); // 镜面高光
	vec3 specular = specularStrength * spec * u_LightColor;
	// Combination
	vec3 result = (ambient + diffuse + specular) * u_Color;

	color = vec4(result, 1.0);
}