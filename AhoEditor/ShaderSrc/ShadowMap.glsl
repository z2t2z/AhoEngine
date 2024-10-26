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

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
layout(std140, binding = 3) uniform SkeletalUBO{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightPV; // ortho * view
	mat4 u_BoneMatrices[MAX_BONES];
};

uniform bool u_IsInstanced;
uniform bool u_IsSkeletal;
uniform mat4 u_Model;

void main() {
	vec4 finalPos = vec4(a_Position, 1.0f);
	if (u_IsSkeletal) {
		mat4 skinningMatrix = mat4(0.0f);
		bool hasInfo = false;
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			int id = a_BoneID[i];
			if (id == -1) {
				continue;
			}
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
		finalPos = skinningMatrix * vec4(a_Position, 1.0f);
	}

	gl_Position = u_LightPV * (u_IsInstanced ? a_InstancedTransform : u_Model) * finalPos;
}


#type fragment
#version 460 core

void main() {
	//gl_FragDepth = gl_FragCoord.z;
}