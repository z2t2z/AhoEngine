#ifndef SAMPLING_GLSL
#define SAMPLING_GLSL

#include "./Random.glsl"
#include "./GlobalVars.glsl"

vec3 SampleCosineHemisphere() {
    float u1 = rand();
    float u2 = rand();
    float r = sqrt(u1);
    float phi = 2.0 * PI * u2;

    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(1 - x * x - y * y);
    return vec3(x, y, z);
}

vec3 SampleUniformHemisphere() {
    vec2 u = vec2(rand(), rand());
    float z = u[0];
    float r = sqrt(1 - sqrt(z));
    float phi = 2 * PI * u[1];
    return vec3(r * cos(phi), r * sin(phi), z);
}

float UniformHemispherePDF() {
    return Inv2PI;
}

vec3 SampleUniformSphere() {
    vec2 u = vec2(rand(), rand());
    float z = 1 - 2 * u[0];
    float r = sqrt(max(0.f, 1 - z*z));
    float phi = 2 * PI * u[1];
    return vec3(r * cos(phi), r * sin(phi), z);
}



#endif