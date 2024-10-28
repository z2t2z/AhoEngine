#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in vec4 a_BoneWeights;
layout(location = 6) in ivec4 a_BoneID;
layout(location = 7) in mat4 a_InstancedTransform;

const int MAX_BONES = 500;
const int MAX_BONE_INFLUENCE = 4;
layout(std140, binding = 3) uniform SkeletalUBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightPV; // ortho * view
	mat4 u_BoneMatrices[MAX_BONES];
};

uniform mat4 u_Model;
uniform bool u_IsInstanced;
uniform int u_BoneOffset;

out vec3 v_FragPos;
out vec3 v_FragPosLight;
out vec3 v_Normal;
out vec3 v_Tangent;
out vec2 v_TexCoords;
out vec3 v_Ndc;
out flat mat3 v_NormalMatrix;

void main() {
	vec3 normal = a_Normal;
	vec3 tangent = a_Tangent;
	mat4 skinningMatrix = mat4(0.0f);
	bool hasInfo = false;
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		int id = a_BoneID[i];
		if (id == -1) {
			continue;
		}
		id += u_BoneOffset;
		if (id >= MAX_BONES) {
			skinningMatrix = mat4(1.0f);
			break;
		}
		hasInfo = true;
		skinningMatrix += a_BoneWeights[i] * u_BoneMatrices[id];
	}
	if (!hasInfo) {
		skinningMatrix = mat4(1.0f);
	}

	mat4 finalModelMat = u_IsInstanced ? u_Model * a_InstancedTransform * skinningMatrix : u_Model * skinningMatrix;

	vec4 PosViewSpace = u_View * finalModelMat * vec4(a_Position, 1.0f);
	v_NormalMatrix = transpose(inverse(mat3(u_View * finalModelMat)));

	v_FragPos = PosViewSpace.xyz;
	gl_Position = u_Projection * PosViewSpace;
	v_TexCoords = a_TexCoords;
	v_Normal = v_NormalMatrix * a_Normal;
	v_Tangent = v_NormalMatrix * a_Tangent;
	v_Ndc = gl_Position.xyz / gl_Position.w;
	v_Ndc = v_Ndc * 0.5f + 0.5f;
	v_FragPosLight = v_FragPosLight;
}

#type fragment
#version 460 core

layout(location = 0) out vec3 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3 g_Albedo;
layout(location = 3) out float g_Depth;
layout(location = 4) out uint g_Entity;

const int MAX_BONES = 500;
const int MAX_BONE_INFLUENCE = 4;
layout(std140, binding = 3) uniform SkeletalUBO{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightPV; // ortho * view
	mat4 u_BoneMatrices[MAX_BONES];
};

in vec3 v_FragPos;
in vec3 v_Ndc;
in vec3 v_FragPosLight;
in vec3 v_Normal;
in vec3 v_Tangent;
in vec2 v_TexCoords;
in flat mat3 v_NormalMatrix;

uniform bool u_HasDiffuse;
uniform bool u_HasNormal;
uniform bool u_IsDebug;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;
uniform mat4 u_Model;
uniform uint u_EntityID;

void main() {
	g_Entity = u_EntityID;
	g_Position = v_FragPos;
	g_Depth = v_FragPos.z;
	g_Albedo = u_HasDiffuse ? texture(u_Diffuse, v_TexCoords).rgb : vec3(0.9f);

	if (!u_HasNormal) {
		g_Normal = normalize(v_Normal);
	}
	else {
		vec3 T = normalize(v_Tangent);
		vec3 N = normalize(v_Normal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T); // NOTE: right-handed
		mat3 TBN = mat3(T, B, N);
		vec3 normalMap = texture(u_Normal, v_TexCoords).rgb;
		normalMap = normalMap * 2.0f - 1.0f;
		g_Normal = normalize(TBN * normalMap);
	}
}