#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    gl_Position = vec4(p, 1.0);
	float x = p.x, y = p.y;
	v_TexCoords = (vec2(x, y) * 0.5) + 0.5;
}

#type fragment
#version 460 core

#include "./Math.glsl"

out vec4 out_Color;
in vec2 v_TexCoords;

uniform sampler2D u_PathTracingAccumulate;
uniform int u_Frame;

vec3 ToneMapping(vec3 c, float limit) {
    return c * 1.0 / (1.0 + Luminance(c) / limit);
}

void main() {
    vec4 accumulate = texture(u_PathTracingAccumulate, v_TexCoords);
    float index = u_Frame;
    out_Color = accumulate / index;
    
    out_Color.rgb = ToneMapping(out_Color.rgb, 1.5);
    out_Color = vec4(pow(out_Color.rgb, vec3(1.0 / 2.2)), 1.0); 
}