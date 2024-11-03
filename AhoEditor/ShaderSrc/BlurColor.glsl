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
out vec3 out_Color;
in vec2 v_TexCoords;

uniform sampler2D u_TexToBlur;

void main() {
    vec2 texelSize = 1.0f / vec2(textureSize(u_TexToBlur, 0));
    vec3 result = vec3(0.0f);
    for (int x = -2; x < 2; x++) {
        for (int y = -2; y < 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_TexToBlur, v_TexCoords + offset).rgb;
        }
    }
    out_Color = result / (4.0 * 4.0);
}