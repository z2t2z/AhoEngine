#type vertex
#version 460 core

#include "Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec4 a_BoneWeights;
layout(location = 5) in ivec4 a_BoneID;

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main() {    
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
	gl_Position = u_Projection * u_View * finalModelMat * vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_Color;
in vec2 v_TexCoords;

void main() {
    out_Color = vec4(1.0, 1.0, 0.0, 1.0); 
}