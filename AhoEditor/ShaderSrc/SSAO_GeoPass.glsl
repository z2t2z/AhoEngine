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
out vec3 v_Tangent;
out vec2 v_TexCoords;

uniform mat4 u_Model;

void main() {
	vec4 PosViewSpace = u_View * u_Model * vec4(a_Position, 1.0f);
	v_FragPos = PosViewSpace.xyz;
	gl_Position = u_Projection * PosViewSpace;
	v_TexCoords = a_TexCoords;
	v_Normal = a_Normal;
	v_Tangent = a_Tangent;
}

#type fragment
#version 460 core

layout(location = 0) out vec3 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3 g_Albedo;
layout(location = 3) out float g_Depth;
layout(std140, binding = 0) uniform UBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view
};

in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_Tangent;
in vec3 v_FragPos;

uniform bool u_HasDiffuse;
uniform bool u_HasNormal;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;
uniform mat4 u_Model;

void main() {
	g_Position = v_FragPos;
	vec4 clipSpcace = u_Projection * vec4(v_FragPos, 1.0f);
	g_Depth = v_FragPos.z;

	if (u_HasDiffuse) {
		g_Albedo = texture(u_Diffuse, v_TexCoords).rgb;
	}
	else {
		g_Albedo = vec3(0.9f);
	}

	mat3 normalMatrix = transpose(inverse(mat3(u_View * u_Model)));
	if (!u_HasNormal) {
		g_Normal = normalize(normalMatrix * v_Normal);
	}
	else {
		vec3 T = normalize(normalMatrix * v_Tangent);
		vec3 N = normalize(normalMatrix * v_Normal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T); // NOTE: right-handed
		mat3 TBN = mat3(T, B, N);
		vec3 normalMap = texture(u_Normal, v_TexCoords).rgb;
		normalMap = normalize(normalMap * 2.0f - 1.0f);
		g_Normal = TBN * normalMap;
	}
}