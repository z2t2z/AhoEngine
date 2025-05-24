#type compute
#version 460

// TODO: Use this binding mechanism
// layout(binding = 0) uniform sampler2D u_EquirectangularMap;

layout(binding = 0, rgba16f) uniform writeonly imageCube o_Cubemap;
layout(local_size_x = 16, local_size_y = 16) in;

uniform sampler2D u_EquirectangularMap;
uniform vec2 u_EquirectangularMapSize;
uniform float u_CubemapFaceSize;

const float PI     = 3.14159265358979323846;
const float TWO_PI = 6.28318530717958647692;

vec3 GetSamplingVector(vec2 uv, uint faceIdx) {
    vec3 ret;
    if (faceIdx == 0)      ret = vec3(1.0,  -uv.y, -uv.x);
    else if (faceIdx == 1) ret = vec3(-1.0, -uv.y,  uv.x);
    else if (faceIdx == 2) ret = vec3(uv.x, 1.0, uv.y);
    else if (faceIdx == 3) ret = vec3(uv.x, -1.0, -uv.y);
    else if (faceIdx == 4) ret = vec3(uv.x, -uv.y, 1.0);
    else if (faceIdx == 5) ret = vec3(-uv.x, -uv.y, -1.0);
    return normalize(ret);
}

void main() {
    uint faceIdx = gl_GlobalInvocationID.z;
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / u_CubemapFaceSize;  // normalized [0,1]
    uv = uv * 2.0 - 1.0; // [0, 1] -> [-1, 1]

    vec3 dir = GetSamplingVector(uv, faceIdx);

    float phi = atan(dir.z, dir.x);
    float theta = acos(clamp(dir.y, -1.0, 1.0));

    vec2 uvEquirect = vec2((phi + PI) / TWO_PI, theta / PI);
    vec4 color = texture(u_EquirectangularMap, uvEquirect);

    imageStore(o_Cubemap, ivec3(gl_GlobalInvocationID), color);
}
