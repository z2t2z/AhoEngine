#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform UBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view
};

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform mat4 u_Model;

void main() {
	vec4 PosViewSpace = u_View * u_Model * vec4(a_Position, 1.0f);
	v_FragPos = PosViewSpace.xyz;
	gl_Position = u_Projection * PosViewSpace;
	v_TexCoords = a_TexCoords;
	mat3 normalMatrix = transpose(inverse(mat3(u_View * u_Model)));
	v_Normal = normalize(normalMatrix * a_Normal);
}

#type fragment
#version 460 core

layout(location = 0) out vec3 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3 g_Albedo;

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_Normal;

uniform sampler2D u_Diffuse;

void main() {
	g_Position = v_FragPos;
	g_Normal = v_Normal;
	g_Albedo = texture(u_Diffuse, v_TexCoords).rgb;
}