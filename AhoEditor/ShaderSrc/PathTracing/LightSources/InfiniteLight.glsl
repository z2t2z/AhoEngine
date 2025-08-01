#ifndef INFINITE_LIGHT_GLSL
#define INFINITE_LIGHT_GLSL

#define OPT_IMAGE_LIGHT

#include "../Math.glsl"

const vec3 uniformSky = vec3(0.529f, 0.808f, 0.922f);
float EnvIntensity = 1.0f;

struct EnvironmentMap {
    sampler2D EnvLight;
    sampler1D Env1DCDF;
    sampler2D Env2DCDF;
    sampler2D Env2DCDF_Reference;
    ivec2 EnvSize;
    float EnvTotalLum;
    float Intensity;
};
uniform EnvironmentMap u_EnvMap;

vec2 DirectionToUV(const vec3 dir) {
    vec2 uv = vec2(0.5f + atan(dir.z, dir.x) * Inv2PI, acos(dir.y) * InvPI);
    return uv;
}
vec3 UVToDirection(const vec2 uv) {
    float phi = uv.x * 2.0 * PI;
    float theta = uv.y * PI;
    return vec3(-cos(phi) * sin(theta), cos(theta), -sin(phi) * sin(theta));
}
// rnd is in [0, 1)
int _BinarySearchCDF1D(float rnd) {
    int l = 0, r = u_EnvMap.EnvSize.x - 1;
    while (l < r) {
        int mid = (l + r) / 2;
        if (texelFetch(u_EnvMap.Env1DCDF, mid, 0).r >= rnd) {
            r = mid;
        } else {
            l = mid + 1;
        }
    }
    return r;
}
// rnd is in [0, 1)
int _BinarySearchCDF2D(int u, float rnd) {
    int height = u_EnvMap.EnvSize.y;
    int l = 0, r = height - 1;
    while (l < r) {
        int mid = (l + r) / 2;
        ivec2 uv = ivec2(u, mid);
        if (texelFetch(u_EnvMap.Env2DCDF, uv, 0).r >= rnd) {
            r = mid;
        } else {
            l = mid + 1;
        }
    }
    return r;
}

// Also correct, from PATH_TRACER, // TODO: needs to check why it is correct
vec2 BinarySearch(float value) {
    ivec2 envMapResInt = u_EnvMap.EnvSize;
    vec2 envMapRes = vec2(envMapResInt.x, envMapResInt.y);
    int lower = 0;
    int upper = envMapResInt.y - 1;
    while (lower < upper)
    {
        int mid = (lower + upper) >> 1;
        if (value < texelFetch(u_EnvMap.Env2DCDF_Reference, ivec2(envMapResInt.x - 1, mid), 0).r)
            upper = mid;
        else
            lower = mid + 1;
    }
    int y = clamp(lower, 0, envMapResInt.y - 1);

    lower = 0;
    upper = envMapResInt.x - 1;
    while (lower < upper)
    {
        int mid = (lower + upper) >> 1;
        if (value < texelFetch(u_EnvMap.Env2DCDF_Reference, ivec2(mid, y), 0).r)
            upper = mid;
        else
            lower = mid + 1;
    }
    int x = clamp(lower, 0, envMapResInt.x - 1);
    return vec2(x, y) / envMapRes;
}

// Sample environment light
vec3 SampleIBL(out float pdf, out vec3 sampleDir) {
    pdf = 0.0;
    int width = u_EnvMap.EnvSize.x;
    int height = u_EnvMap.EnvSize.y;
    if (width == 0 || height == 0) {
        return vec3(0.0, 0.0, 0.0);
    }

    vec2 rnd2 = vec2(rand(), rand());
    int u = _BinarySearchCDF1D(rnd2.x);
    int v = _BinarySearchCDF2D(u, rnd2.y);

    vec2 uv = vec2(u, v) / vec2(width, height);

    vec3 color = texture(u_EnvMap.EnvLight, uv).rgb;
    pdf = Luminance(color) / u_EnvMap.EnvTotalLum;
    
    float phi = uv.x * 2.0 * PI;
    float theta = uv.y * PI;
    sampleDir = vec3(-cos(phi) * sin(theta), cos(theta), -sin(phi) * sin(theta));

    // https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Transforming_between_Distributions#sec:mc-transform-multiple-dimensions
    pdf = (pdf * width * height) / (2.0 * PI * PI * sin(theta));
    return color;
}

vec4 EvalEnvMap(const vec3 dir) {
    float theta = acos(dir.y);
    vec2 uv = vec2(0.5f + atan(dir.z, dir.x) * Inv2PI, theta * InvPI);
    vec3 L = texture(u_EnvMap.EnvLight, uv).rgb;
    float pdf = Luminance(L) / u_EnvMap.EnvTotalLum;
    pdf = (pdf * u_EnvMap.EnvSize.x * u_EnvMap.EnvSize.y) / (2.0f * PI * PI * sin(theta));
    return vec4(EnvIntensity * L, pdf);
}

#endif