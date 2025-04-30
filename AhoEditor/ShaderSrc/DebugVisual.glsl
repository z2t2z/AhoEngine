#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;
out float v_Intensity;

#include "Common/UniformBufferObjects.glsl"

const mat4 RotateMatrix0 = mat4(1.0f);
const mat4 RotateMatrix2 = mat4(
    1.0,  0.0,  0.0,  0.0,  // 绕 X 轴旋转 90°
    0.0,  0.0,  1.0,  0.0,
    0.0, -1.0,  0.0,  0.0,
    0.0,  0.0,  0.0,  1.0
);
const mat4 RotateMatrix1 = mat4(
    0.0,  1.0,  0.0,  0.0,  // 绕 Z 轴旋转 90°
   -1.0,  0.0,  0.0,  0.0,
    0.0,  0.0,  1.0,  0.0,
    0.0,  0.0,  0.0,  1.0
);
uniform int u_RotateIndex = 0;
const mat4 RotateMatrixs[3] = {
	RotateMatrix0,
	RotateMatrix1,
	RotateMatrix2
};
const mat4 Scale = mat4(
    1.0,  0.0,  0.0,  0.0,  // 绕 Z 轴旋转 90°
    0.0,  1.0,  0.0,  0.0,
    0.0,  0.0,  1.0,  0.0,
    0.0,  0.0,  0.0,  1.0
);

mat4 ConstructModelMatrix(PointLight light) {
	vec3 pos = light.position.xyz;
	float radius = light.position.w;
	mat4 model = Scale;
	model[0][0] = model[1][1] = model[2][2] = radius;
	model[3][0] = pos.x;
	model[3][1] = pos.y;
	model[3][2] = pos.z;
	return model * RotateMatrixs[u_RotateIndex];
}

void main() {
	PointLight light = u_PointLight[0];
	v_Intensity = 0.0;
	mat4 model = ConstructModelMatrix(light);
	gl_Position = u_Projection * u_View * model * vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
layout(location = 0) out vec4 out_Color;

in vec2 v_TexCoords;
in float v_Intensity;

void main() {
	out_Color = vec4(1.0, 1.0, 1.0, v_Intensity > 0 ? 1.0 : 0.0); 
}
