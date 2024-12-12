layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

const int MAX_LIGHT_CNT = 10;
layout(std140, binding = 1) uniform LightUBO {
	mat4 u_LightPV[MAX_LIGHT_CNT];
	vec4 u_LightPosition[MAX_LIGHT_CNT];
	vec4 u_LightColor[MAX_LIGHT_CNT];
	ivec4 u_Info[MAX_LIGHT_CNT]; // Enabled status; Light type; ...
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
