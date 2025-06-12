#type vertex
#version 460 core

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    gl_Position = vec4(p, 1.0);
}

#type fragment
#version 460 core

layout(location = 0) out vec4 out_Color;

uniform sampler2D u_SelectedDepth;
uniform sampler2D u_Scene;
uniform sampler2D u_PT_Scene;
uniform int u_RenderMode = 0; // 0: Deferred, 1: PathTracing
uniform vec3 u_OutlineColor = vec3(1, 1, 0);

const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = { 0, 1, 0, -1};

void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 imageSiz = textureSize(u_SelectedDepth, 0);
    bool is_outline = false;
    for (int i = 0; i < 4; i++) {
        ivec2 ncoords = ivec2(coords.x + dx[i], coords.y + dy[i]);
        if (ncoords.x >= 0 && ncoords.x < imageSiz.x && ncoords.y >= 0 && ncoords.y < imageSiz.y) {
            float d1 = texelFetch(u_SelectedDepth, ncoords, 0).r;
            if (d1 != 1.0) {
                is_outline = true;
                break;
            }
        }
    }
    float d = texelFetch(u_SelectedDepth, coords, 0).r;
    if (d != 1.0) {
        is_outline = false;
    }
    out_Color = is_outline ? vec4(u_OutlineColor, 1.0) : texelFetch(u_RenderMode == 0 ? u_Scene : u_PT_Scene, coords, 0).rgba;
}