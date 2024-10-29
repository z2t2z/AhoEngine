#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in vec4 a_BoneWeights;
layout(location = 6) in ivec4 a_BoneID;

const int MAX_LIGHT_CNT = 10;
layout(std140, binding = 1) uniform LightUBO {
	mat4 u_LightPV[MAX_LIGHT_CNT];
	vec4 u_LightPosition[MAX_LIGHT_CNT];
	vec4 u_LightColor[MAX_LIGHT_CNT];
	ivec4 u_Info[MAX_LIGHT_CNT]; // Enabled status; Light type; ...
};

// For animation, read by vertices
const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONES_CNT = 200;
layout(std140, binding = 3) uniform AnimationUBO {
	mat4 u_BoneMatrices[MAX_BONES_CNT];
};

uniform bool u_IsInstanced;
uniform int u_BoneOffset;
uniform mat4 u_Model;

void main() {
	vec4 finalPos = vec4(a_Position, 1.0f);
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
	} else {
		finalPos = skinningMatrix * vec4(a_Position, 1.0f);
	}
	gl_Position = u_LightPV[0] * u_Model * finalPos;
}


#type fragment
#version 460 core

void main() {
	//gl_FragDepth = gl_FragCoord.z;
}