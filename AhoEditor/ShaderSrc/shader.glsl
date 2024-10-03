#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

void main() {
	v_Position = vec3(u_Model * vec4(a_Position, 1.0));
	gl_Position = u_Projection * u_View * u_Model * vec4(v_Position, 1.0);
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoords;

uniform vec3 u_ViewPosition;
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform vec3 u_Color;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;

// Blinn-Phong
void main() {
	// Ambient
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * vec3(1.0);
	// Diffuse
	vec3 norm = normalize(v_Normal);         // 法线归一化
	vec3 lightDir = normalize(-u_LightPosition + v_Position); // 计算光源方向
	float diff = max(dot(norm, lightDir), 0.0);    // 漫反射分量
	vec3 diffuse = diff * u_LightColor;
	// Specular
	float specularStrength = 1.0;
	vec3 viewDir = normalize(-u_ViewPosition + v_Position);  // 视线方向
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong中的半程向量
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32); // 镜面高光
	vec3 specular = specularStrength * spec * u_LightColor;
	// Combination
	vec3 result = (ambient + diffuse + specular) * u_Color;

	//color = vec4(v_TexCoords.x, v_TexCoords.y, 0.0, 1.0);
	color = texture(u_Diffuse, v_TexCoords);
	//color = vec4(0.5, 0.1, 0.2, 1.0);
}