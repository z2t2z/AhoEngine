#type vertex
#version 460 core

#include "Common/UniformBufferObjects.glsl"

out vec3 v_nearP;
out vec3 v_farP;

vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

// To world space
vec3 NDCToWorld(vec3 p) {
    vec4 pos = u_ViewInv * u_ProjectionInv * vec4(p, 1.0);
    if (pos.w != 0.0) {
        pos /= pos.w;
    }
    return pos.xyz;
}

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    v_nearP = NDCToWorld(vec3(p.xy, -1.0));
    v_farP = NDCToWorld(vec3(p.xy, 1.0));
    gl_Position = vec4(p, 1.0);
}

#type fragment
#version 460 core

#include "Common/UniformBufferObjects.glsl"

layout(location = 0) out vec4 out_Color;

in vec3 v_nearP;
in vec3 v_farP;

vec4 genGrid(vec3 worldPos, float scale) {
    vec2 coord = worldPos.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    float fac = 0.5 / scale;
    if (worldPos.x > -0.5 * minimumx && worldPos.x < 0.5 * minimumx) {
        color.z = 1.0;
    }
    // x axis
    if (worldPos.z > -0.5 * minimumz && worldPos.z < 0.5 * minimumz) {
        color.x = 1.0;
    }
    return color;
}

float Grid(vec2 uv) {
    vec2 derivative = fwidth(uv);
    uv = fract(uv - 0.5); //中心对齐
    uv = abs(uv - 0.5);
    uv = uv / derivative;
    float min_value = min(uv.x, uv.y);
    float grid = 1.0 - min(min_value, 1.0);
    return grid;
}

// Set as a constant for now
const float camFarPlane = 1000.0f;

uniform sampler2D u_Depth;

void main() {
    float t = -v_nearP.y / (v_farP.y - v_nearP.y);
    vec3 worldPos = v_nearP + t * (v_farP - v_nearP);
    float fromOrigin = abs(u_ViewPosition.y);// / camFarPlane; // [0, 1]

    vec4 s = genGrid(worldPos, 1.0) * mix(1.0, 0.0, min(1.0, fromOrigin / 100));
    vec4 m = genGrid(worldPos, 0.1) * mix(1.0, 0.0, min(1.0, fromOrigin / 200));
    vec4 l = genGrid(worldPos, 0.01) * mix(1.0, 0.0, min(1.0, fromOrigin / 300));

    float ss = Grid(worldPos.xy) * mix(1.0, 0.0, min(1.0, fromOrigin / 100));
    float sm = Grid(worldPos.xy * 0.1) * mix(1.0, 0.0, min(1.0, fromOrigin / 400));
    float sl = Grid(worldPos.xy * 0.01) * mix(1.0, 0.0, min(1.0, fromOrigin / 800));

    out_Color = (s + m + l) * float(t > 0);
    // out_Color = vec4(1.0, 0.0, 0.0, step(0, t) * (ss + sm + sl));
}