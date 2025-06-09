#ifndef UBO_GLSL
#define UBO_GLSL

#include "LightStructs.glsl"

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	mat4 u_ProjView;
	vec4 u_ViewPosition;
};

// TODO: Delete this
const int MAX_LIGHT_CNT = 5;
layout(std140, binding = 1) uniform DirLight {
	DirectionalLight u_DirectionalLight[MAX_LIGHT_CNT];
};

// For animation, read by vertices
const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONES_CNT = 200;
layout(std140, binding = 2) uniform AnimationUBO {
	mat4 u_BoneMatrices[MAX_BONES_CNT];
};

// For ssao pass
// const int SAMPLES_CNT = 64;
// layout(std140, binding = 2) uniform RandomKernelUBO {
// 	vec4 u_Samples[SAMPLES_CNT];
// 	vec4 u_RdInfo; // width, height, radius, bias
// };

#endif
