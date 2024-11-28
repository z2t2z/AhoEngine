#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

// For animation, read by vertices
const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONES_CNT = 200;
layout(std140, binding = 3) uniform AnimationUBO {
	mat4 u_BoneMatrices[MAX_BONES_CNT];
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

void main() {
	vec3 normal = a_Normal;
	vec3 tangent = a_Tangent;
	mat4 skinningMatrix = mat4(0.0f);
	bool hasAnimationInfo = false;
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
		int id = a_BoneID[i];
		if (id == -1) {
			continue;
		}
		id += u_BoneOffset;
		if (id >= MAX_BONES_CNT) {
			skinningMatrix = mat4(1.0f);
			break;
		}
		hasAnimationInfo = true;
		skinningMatrix += a_BoneWeights[i] * u_BoneMatrices[id];
	}
	if (!hasAnimationInfo) {
		skinningMatrix = mat4(1.0f);
	}

	mat4 finalModelMat = u_Model * skinningMatrix;

	// Positions are in view space
	vec4 PosViewSpace = u_View * finalModelMat * vec4(a_Position, 1.0f);

	// Normals are in world space
	mat3 v_NormalMatrix = transpose(inverse(mat3(finalModelMat)));

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

layout(location = 0) out vec4 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3 g_Albedo;
layout(location = 3) out vec4 g_PBR;
layout(location = 4) out uint g_Entity;

in vec3 v_FragPos;
in vec3 v_Ndc;
in vec3 v_FragPosLight;
in vec3 v_Normal;
in vec3 v_Tangent;
in vec2 v_TexCoords;

uniform bool u_HasAlbedo;
uniform bool u_HasNormal;
uniform bool u_HasMetallic;
uniform bool u_HasRoughness;
uniform bool u_HasAO;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetalicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

uniform vec3 u_RawAlbedo = vec3(0.98f);
uniform float u_Metalic = 0.05f;
uniform float u_Roughness = 0.9f;

uniform uint u_EntityID;

void main() {
	g_Entity = u_EntityID;
	g_Position = vec4(v_FragPos, 1.0f);

	// PBR
	g_Albedo = u_HasAlbedo ? texture(u_AlbedoMap, v_TexCoords).rgb : u_RawAlbedo;
	g_PBR.r = u_HasMetallic ? 1.0f - texture(u_MetalicMap, v_TexCoords).r : u_Metalic;
	g_PBR.g = u_HasRoughness ? texture(u_RoughnessMap, v_TexCoords).r : u_Roughness;
	g_PBR.b = u_HasAO ? texture(u_AOMap, v_TexCoords).r : -1.0f;

	if (!u_HasNormal) {
		g_Normal = normalize(v_Normal);
	}
	else {
		vec3 T = normalize(v_Tangent);
		vec3 N = normalize(v_Normal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T); // NOTE: right-handed
		mat3 TBN = mat3(T, B, N);
		vec3 normalMap = texture(u_NormalMap, v_TexCoords).rgb;
		normalMap = normalMap * 2.0f - 1.0f;
		g_Normal = normalize(TBN * normalMap);
	}
}