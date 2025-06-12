#type vertex
#version 460 core

#include "../Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

uniform int u_BoneOffset;
uniform mat4 u_Model;

void main() {
	vec4 finalPos = vec4(a_Position, 1.0f);
	mat4 skinningMatrix = mat4(1.0f);
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
	if (hasAnimationInfo) {
		finalPos = skinningMatrix * vec4(a_Position, 1.0f);
	}
	gl_Position = u_Projection * u_View * u_Model * finalPos;
}

#type fragment
#version 460 core

void main() {
	gl_FragDepth = gl_FragCoord.z;
}