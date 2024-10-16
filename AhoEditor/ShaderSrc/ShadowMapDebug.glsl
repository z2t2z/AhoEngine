#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform CameraData{
	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_LightViewMatrix; // ortho * view
	vec4 u_ViewPosition;
};

out vec2 TexCoords;

void main() {
    gl_Position = vec4(a_Position, 1.0);
    TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
in vec2 TexCoords;

out vec4 color;

uniform sampler2D u_DepthMap;
uniform sampler2D u_Diffuse;
uniform float u_Near;
uniform float u_Far;
uniform bool u_IsPerspective;

void main() {
    float depth = texture(u_DepthMap, TexCoords).r; // [0, 1] 
    // depth = (2.0 * u_Near * u_Far) / (u_Far + u_Near - depth * (u_Far - u_Near));
    depth = (depth - 0.1f) / (50.0f - 0.1f);
    // vec3 diffuse = texture(u_Diffuse, TexCoords).rgb;
    color = vec4(vec3(depth), 1.0f); // assumue a orthographic projection is used for now
    // color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}