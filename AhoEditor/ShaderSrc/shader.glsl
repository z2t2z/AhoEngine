#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140) uniform CameraData{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_Model;
	vec3 u_ViewPosition;
	float padding0;         // 4 bytes (for alignment)
	vec3 u_LightPosition;
	float padding1;         // 4 bytes (for alignment)
	vec3 u_LightColor;
	float padding2;         // 4 bytes (for alignment)
};

out vec3 v_Position;
out vec3 v_PositionTangent;
out vec3 v_Normal;
out vec2 v_TexCoords;

out vec3 v_LightPos;	
out vec3 v_ViewPos;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
	v_Position = vec3(u_Model * vec4(a_Position, 1.0));
	v_TexCoords = a_TexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
	vec3 T = normalize(normalMatrix * a_Tangent);
	vec3 N = normalize(normalMatrix * a_Normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	v_LightPos = TBN * u_LightPosition;
	v_ViewPos = TBN * u_ViewPosition;
	v_PositionTangent = TBN * v_Position;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 color;

layout(std140) uniform CameraData{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_Model;
	vec3 u_ViewPosition;
	float padding0;         // 4 bytes (for alignment)
	vec3 u_LightPosition;
	float padding1;         // 4 bytes (for alignment)
	vec3 u_LightColor;
	float padding2;         // 4 bytes (for alignment)
};

in vec3 v_Position;
in vec3 v_PositionTangent;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_LightPos;
in vec3 v_ViewPos;

// uniform vec3 u_LightColor;
uniform vec3 u_Color;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;

// Blinn-Phong
void main() {
	// Ambient
	float ambientStrength = 0.2;
	vec3 rawColor = texture(u_Diffuse, v_TexCoords).rgb;
	vec3 ambient = ambientStrength * rawColor;

	// Diffuse
	vec3 normal = texture(u_Normal, v_TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 lightDir = normalize(v_LightPos - v_PositionTangent);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * rawColor;

	// Specular
	float specularStrength = 1.0;
	vec3 viewDir = normalize(-v_ViewPos + v_PositionTangent);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * u_LightColor;
	float specStrength = 0.2;
	specular = specStrength * specular;
	// Combination
	vec3 result = (ambient + diffuse + specular);
	color = vec4(result, 1.0);
	// color = vec4(1.0);
}