#ifndef INFINITE_LIGHT_GLSL
#define INFINITE_LIGHT_GLSL

#define OPT_IMAGE_LIGHT

#include "./Math.glsl"
#include "./IntersectionTest.glsl"

struct EnvironmentMap {
    sampler2D EnvLight;
    sampler1D Env1DCDF;
    sampler2D Env2DCDF;
    ivec2 EnvSize;
    float EnvTotalLum;
};
uniform EnvironmentMap u_EnvMap;

vec2 DirectionToUV(const vec3 dir) {
    vec2 uv = vec2(0.5f + atan(dir.z, dir.x) * Inv2PI, acos(dir.y) * InvPI);
    return uv;
}
vec3 UVToDirection(const vec2 uv) {
    float phi = uv.x;
    float theta = uv.y;
    return vec3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));
}

int _BinarySearchCDF1D(float rnd) {
    int l = 0, r = u_EnvMap.EnvSize.x - 1;
    while (l < r) {
        int mid = int(l + r) / 2;
        if (rnd >= texelFetch(u_EnvMap.Env1DCDF, mid, 0).r) {
            r = mid;
        } else {
            l = mid + 1;
        }
    }
    return r;
}
int _BinarySearchCDF2D(int u, float rnd) {
    int height = u_EnvMap.EnvSize.y;
    int l = 0, r = height - 1;
    ivec2 uv = ivec2(u, 0);
    while (l < r) {
        int mid = (l + r) / 2;
        uv.y = mid;
        if (rnd >= texelFetch(u_EnvMap.Env2DCDF, uv, 0).r) {
            r = mid;
        } else {
            l = mid + 1;
        }
    }
    return r;
}

// Sample environment light
// Not sure if correct
vec3 SampleIBL(inout float pdf, vec3 from) {
    int width = u_EnvMap.EnvSize.x;
    int height = u_EnvMap.EnvSize.y;
    if (width == 0 || height == 0) {
        return vec3(0.0, 0.0, 0.0);
    }
    vec2 rnd = vec2(rand(), rand());
    int u = _BinarySearchCDF1D(rnd.x);
    int v = _BinarySearchCDF2D(u, rnd.y);

    float pu = texelFetch(u_EnvMap.Env1DCDF, u, 0).r;
    pu -= u > 0 ? texelFetch(u_EnvMap.Env1DCDF, u - 1, 0).r : 0.0f;
    float pv = texelFetch(u_EnvMap.Env2DCDF, ivec2(u, v), 0).r;
    pv -= v > 0 ? texelFetch(u_EnvMap.Env2DCDF, ivec2(u, v - 1), 0).r : 0.0f;

    float testpdf = pu * pv;

    vec2 uv = vec2(u, v) / vec2(width, height);
    vec3 dir = UVToDirection(uv);
    
    if (!VisibilityTest(from, from + 10000.0 * dir)) {
        return vec3(0.0, 0.0, 0.0);
    }

    vec3 color = texture(u_EnvMap.EnvLight, uv).rgb;
    
    pdf = Luminance(color) / u_EnvMap.EnvTotalLum;
    // if (abs(testpdf - pdf) >= 0.0000001f) {
    //     return vec3(1.0, 0.0, 0.0);
    // }

    float phi = uv.x * 2.0 * PI;
    float theta = uv.y * PI;

    pdf = (pdf * width * height) / (2 * PI * PI * sin(theta));
    return color;
}

const vec3 uniformSky = vec3(0.529f, 0.808f, 0.922f);
float Intensity = 2.0f;
vec3 QueryInfiniteImageLight(const Ray ray) {
    if (u_EnvMap.EnvSize.x == 0 || u_EnvMap.EnvSize.y == 0) {
        return uniformSky;
    }
    vec2 uv = DirectionToUV(ray.direction);
    vec3 color = texture(u_EnvLight, uv).rgb * abs(SinTheta(ray.direction));
    return color / PI;
}

vec3 EvalEnvMap(const Ray ray, inout float pdf) {
    vec3 L = QueryInfiniteImageLight(ray);
    pdf = Luminance(L) / u_EnvMap.EnvTotalLum;
    return L;
}

vec3 SampleInfiniteLight(Ray ray) {
#ifdef OPT_IMAGE_LIGHT
    return Intensity * QueryInfiniteImageLight(ray);
#endif
    return uniformSky;
}



#endif