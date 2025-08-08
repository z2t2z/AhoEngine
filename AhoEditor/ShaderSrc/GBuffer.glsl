#type vertex
#version 460 core

#include "Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

uniform mat4 u_Model;
uniform bool u_IsInstanced;
uniform int u_BoneOffset;

out vec3 v_FragPos;
out vec3 v_FragPosLight;
out vec3 v_Normal;
out vec3 v_Tangent;
out vec2 v_TexCoords;

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

	// Transform to view space
	mat4 transform = u_View * u_Model * skinningMatrix;
	vec4 PosViewSpace = transform * vec4(a_Position, 1.0f);
	mat3 v_NormalMatrix = transpose(inverse(mat3(transform)));

	v_FragPos = PosViewSpace.xyz;
	// v_FragPos = (u_Model * skinningMatrix * vec4(a_Position, 1.0)).xyz;
	gl_Position = u_Projection * PosViewSpace;
	v_TexCoords = a_TexCoords;
	v_Normal = v_NormalMatrix * a_Normal;
	v_Tangent = v_NormalMatrix * a_Tangent;
	v_FragPosLight = v_FragPosLight;
}

#type fragment
#version 460 core

layout(location = 0) out vec3 g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3 g_Albedo;
layout(location = 3) out vec3 g_PBR;

in vec3 v_FragPos;
in vec3 v_FragPosLight;
in vec3 v_Normal;
in vec3 v_Tangent;
in vec2 v_TexCoords;

uniform bool u_HasAlbedo= false;
uniform bool u_HasNormal= false;
uniform bool u_HasMetallic= false;
uniform bool u_HasRoughness= false;
uniform bool u_HasAO = false;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

uniform vec3 u_Albedo = vec3(1.0);
uniform float u_Metallic = 0.95f;
uniform float u_Roughness = 0.0f;


vec3 LessThan(vec3 f, float value) {
    return vec3(
        (f.x < value) ? 1.f : 0.f,
        (f.y < value) ? 1.f : 0.f,
        (f.z < value) ? 1.f : 0.f);
}
vec3 SRGBToLinear(vec3 rgb) {
    rgb = clamp(rgb, 0.f, 1.f);
    return mix(
        pow((rgb + 0.055f) / 1.055f, vec3(2.4f)),
        rgb / 12.92f,
        LessThan(rgb, 0.04045f)
    );
}


void main() {
	g_Position = v_FragPos;
	g_Albedo = u_HasAlbedo ? texture(u_AlbedoMap, v_TexCoords).rgb : u_Albedo;
	// g_Albedo = SRGBToLinear(g_Albedo); // Convert to linear space
	
	// PBR
	g_PBR.r = u_HasMetallic ? texture(u_MetallicMap, v_TexCoords).r : u_Metallic;
	g_PBR.g = u_HasRoughness ? texture(u_RoughnessMap, v_TexCoords).r : u_Roughness;
	g_PBR.b = u_HasAO ? texture(u_AOMap, v_TexCoords).r : 0.0f;

	vec3 normal = normalize(v_Normal);
	if (!u_HasNormal) {
		g_Normal = normal;
	} else {
		vec3 T = normalize(v_Tangent);
		vec3 N = normal;
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T); // NOTE: right-handed
		mat3 TBN = mat3(T, B, N);
		vec3 normalMap = texture(u_NormalMap, v_TexCoords).rgb;
		normalMap = normalize(normalMap * 2.0f - 1.0f); // rgb8
		g_Normal = normalize(TBN * normalMap);
	}
}