#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

layout(std140) uniform CameraData{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_LightViewMatrix;
	vec3 u_ViewPosition;
	float padding0;         // 4 bytes (for alignment)
	vec3 u_LightPosition;
	float padding1;         // 4 bytes (for alignment)
	vec3 u_LightColor;
	float padding2;         // 4 bytes (for alignment)
};

uniform mat4 u_Model;

void main() {
	gl_Position = u_LightViewMatrix * u_Model * vec4(a_Position, 1.0f);
}


#type fragment
#version 460 core

void main() {
	//gl_FragDepth = gl_FragCoord.z;
}