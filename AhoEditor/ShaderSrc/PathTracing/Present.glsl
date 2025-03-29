#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
    gl_Position = vec4(a_Position, 1.0f);
    v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_Color;
in vec2 v_TexCoords;

uniform sampler2D u_PathTracingAccumulate;
uniform int u_Frame;

void main() {
    vec4 accumulate = texture(u_PathTracingAccumulate, v_TexCoords);
    float index = u_Frame;
    out_Color = accumulate / index;
    vec3 gamma = vec3(2.2, 2.2, 2.2);

    out_Color = vec4(pow(vec3(out_Color), (1.0 / gamma)), 1.0); 
}