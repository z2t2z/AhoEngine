#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;
out vec2 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

void main() {
	v_Position = vec3(u_Model * vec4(a_Position, 1.0));
	gl_Position = u_Projection * u_View * u_Model * vec4(v_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec3 v_Position;

uniform int u_ID;

vec4 ConvertToVec4(uint value) {
	float r = float(value & 0xFF) / 255.0;
	float g = float((value >> 8) & 0xFF) / 255.0;
	float b = float((value >> 16) & 0xFF) / 255.0;
	float a = float((value >> 24) & 0xFF) / 255.0;
	vec4 color = vec4(r, g, b, a);
	return color;
}

void main() {
	color = vec4(
		float(u_ID & 0xFF) / 255.0,              // R
		float((u_ID >> 8) & 0xFF) / 255.0,       // G
		float((u_ID >> 16) & 0xFF) / 255.0,      // B
		float((u_ID >> 24) & 0xFF) / 255.0       // A
	);
}