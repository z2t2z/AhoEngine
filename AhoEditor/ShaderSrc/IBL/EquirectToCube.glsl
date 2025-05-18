#type compute
#version 460

layout(binding = 0) uniform sampler2D u_EquirectangularMap;
layout(binding = 1, rgba32f) uniform writeonly imageCube o_Cubemap;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

vec3 GetSamplingVector(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)     ret = vec3(1.0,  uv.y, -uv.x);
    else if(gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y,  uv.x);
    else if(gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
    else if(gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
    else if(gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
    else if(gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
    return normalize(ret);
}

const float PI     = 3.14159265358979323846;
const float TWO_PI = 6.28318530717958647692;

void main() {
    vec2 uv = gl_GlobalInvocationID.xy / vec2(imageSize(u_EquirectangularMap));
    vec3 dir = GetSamplingVector(uv);

    float phi = atan(dir.z, dir.x);
    float theta = acos(dir.y);
    vec2 uvEquirect = vec2(phi / TWO_PI, theta / PI);
    vec4 color = texture(u_EquirectangularMap, uvEquirect);

    imageStore(o_Cubemap, ivec3(gl_GlobalInvocationID), color);
}
