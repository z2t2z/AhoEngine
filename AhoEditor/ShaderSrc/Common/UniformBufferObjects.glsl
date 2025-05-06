#ifndef UBO_GLSL
#define UBO_GLSL

#include "LightStructs.glsl"

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	mat4 u_ViewProj;
	vec4 u_ViewPosition;
};

// TODO: support multiple light types 
const int MAX_LIGHT_CNT = 5;
layout(std140, binding = 1) uniform LightUBO {
	PointLight u_PointLight[MAX_LIGHT_CNT];
	DirectionalLight u_DirLight[MAX_LIGHT_CNT];
	AreaLight u_AreaLight[MAX_LIGHT_CNT];
	uvec4 u_LightCount; // Point, directional, spot, area
};

// For ssao pass
const int SAMPLES_CNT = 64;
layout(std140, binding = 2) uniform RandomKernelUBO {
	vec4 u_Samples[SAMPLES_CNT];
	vec4 u_RdInfo; // width, height, radius, bias
};

// For animation, read by vertices
const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONES_CNT = 200;
layout(std140, binding = 3) uniform AnimationUBO {
	mat4 u_BoneMatrices[MAX_BONES_CNT];
};


#endif
