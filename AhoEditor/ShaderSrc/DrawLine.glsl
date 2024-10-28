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
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0f);
}

#type fragment
#version 460 core
out vec4 out_color;

void main() {
    out_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}