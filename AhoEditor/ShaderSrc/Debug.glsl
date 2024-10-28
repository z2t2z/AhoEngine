#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

const int MAX_BONES_CNT = 500;
layout(std140, binding = 4) uniform SkeletonUBO {
	mat4 u_BoneMatrices[MAX_BONES_CNT];
	uint u_BoneEntityID[MAX_BONES_CNT];
};

uniform mat4 u_Model;
uniform bool u_IsInstanced;
uniform int u_BoneOffset = 0;

void main() {
	mat4 finalModelMat;
	if (u_IsInstanced) {
		int instanceID = gl_InstanceID;
		finalModelMat = u_Model * u_BoneMatrices[gl_InstanceID + u_BoneOffset];
	} else {
		finalModelMat = u_Model;
	}
	vec4 PosViewSpace = u_View * finalModelMat * mat4(100.0f) * vec4(a_Position, 1.0f);
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