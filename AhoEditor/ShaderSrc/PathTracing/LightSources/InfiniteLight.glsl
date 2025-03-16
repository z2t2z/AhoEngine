#ifndef INFINITE_LIGHT_GLSL
#define INFINITE_LIGHT_GLSL

// Uniform infinite light
const vec3 uniformSky = vec3(0.529f, 0.808f, 0.922f);
vec3 SampleInfiniteLight(Ray ray) {
    return uniformSky;
}



#endif