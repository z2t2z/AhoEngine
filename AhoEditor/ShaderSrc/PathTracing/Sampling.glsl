#ifndef SAMPLING_GLSL
#define SAMPLING_GLSL

#include "./Random.glsl"
#include "./GlobalVars.glsl"

vec3 SampleCosineHemisphere() {
    vec2 u = vec2(rand(), rand());
    float r = sqrt(u.x);
    float phi = 2.0f * PI * u.y;
    float x = r * cos(phi);
    float y = r * sin(phi);
    // float z = sqrt(1.0f - r * r);
    float z = sqrt(max(0.0, 1.0 - x * x - y * y));
    return vec3(x, z, y);
}

float CosineHemispherePDF(vec3 dir) {
    return dir.y / PI;
}


vec3 SampleUniformHemisphere() {
    vec2 u = vec2(rand(), rand());
    // u.x, u.y in [0, 1]
    float phi = 2.0f * PI * u.y;
    // 这里令 cosθ = 1 - u.x
    float cosTheta = 1.0f - u.x;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float z = cosTheta; // 保证 z >= 0

    return vec3(x, z, y);
}

float UniformHemispherePDF(vec3 dir = vec3(0.0, 0.0, 0.0)) {
    return Inv2PI;
}

vec3 SampleUniformSphere() {
    vec2 u = vec2(rand(), rand());
    float z = 1 - 2 * u[0];
    float r = sqrt(max(0.f, 1 - z*z));
    float phi = 2 * PI * u[1];
    return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 SampleGGXVNDF(vec3 V, float ax, float ay, float r1, float r2) {
    vec3 Vh = normalize(vec3(ax * V.x, ay * V.y, V.z));
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1, 0, 0);
    vec3 T2 = cross(Vh, T1);

    float r = sqrt(r1);
    float phi = 2.0 * PI * r2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

    vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

    vec3 H = normalize(vec3(ax * Nh.x, max(0.0, Nh.z), ay * Nh.y));
    return H;
}


#endif