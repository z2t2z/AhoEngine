#type vertex
#version 460 core

#include "../Common/UniformBufferObjects.glsl"

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;

void main() {
	mat4 transform = u_View * u_Model;
	vec4 PosViewSpace = transform * vec4(a_Position, 1.0f);
	gl_Position = u_Projection * PosViewSpace;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 outColor;

uniform vec3 probeDebugColor = vec3(1, 1, 1);

void main() {
    outColor = vec4(probeDebugColor, 1);
}