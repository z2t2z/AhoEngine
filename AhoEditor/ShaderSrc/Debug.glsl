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
layout(std140, binding = 3) uniform SkeletalUBO{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightPV; // ortho * view
	mat4 u_BoneMatrices[MAX_BONES];
};

uniform mat4 u_Model;
uniform bool u_IsInstanced;
uniform int u_BoneOffset;

void main() {
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
	gl_Position = u_Projection * PosViewSpace;
}

#type fragment
#version 460 core
layout(location = 0) out uint g_Entity;
layout(location = 1) out vec4 out_Color;

uniform uint u_EntityID;

void main() {
	g_Entity = u_EntityID;
	out_Color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}