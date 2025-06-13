#type vertex
#version 460 core

#include "../Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

uniform mat4 u_Model;
uniform bool u_IsInstanced;
uniform int u_BoneOffset;

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
	gl_Position = u_Projection * PosViewSpace;
}

#type fragment
#version 460 core

layout(location = 0) out uint o_ObjectID;

uniform uint u_ObjectID;

void main() {
    o_ObjectID = u_ObjectID;
}