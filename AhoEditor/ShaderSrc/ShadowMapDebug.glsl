#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

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
uniform float u_Near;
uniform float u_Far;
uniform bool u_IsPerspective;

void main() {
    float depth = texture(u_DepthMap, TexCoords).r; // [0, 1] 
    //depth = depth * 2.0f - 1.0f; // [-1, 1]
    // depth = (2.0 * u_Near * u_Far) / (u_Far + u_Near - depth * (u_Far - u_Near));
    color = vec4(vec3(depth * depth), 1.0f); // assumue a orthographic projection is used for now
}