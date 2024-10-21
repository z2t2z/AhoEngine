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
out float out_Depth;
in vec2 v_TexCoords;

uniform int u_MipmapLevels;
uniform sampler2D u_Depth;

void main() {
    ivec2 texelPos = ivec2(gl_FragCoord.xy);
    if (u_MipmapLevels == 0) {
        out_Depth = texelFetch(u_Depth, gl_FragCoord.xy, 0).r;
        return;
    }
    float d0 = texelFetch(u_Depth, gl_FragCoord.xy * 2, u_MipmapLevels - 1).r;
    float d1 = texelFetch(u_Depth, gl_FragCoord.xy * 2 + ivec2(1, 0), u_MipmapLevels - 1).r;
    float d2 = texelFetch(u_Depth, gl_FragCoord.xy * 2 + ivec2(0, 1), u_MipmapLevels - 1).r;
    float d3 = texelFetch(u_Depth, gl_FragCoord.xy * 2 + ivec2(1, 1), u_MipmapLevels - 1).r;
    out_Depth = min(min(d0, d1), min(d2, d3));
}