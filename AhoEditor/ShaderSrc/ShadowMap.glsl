#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform UBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view
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