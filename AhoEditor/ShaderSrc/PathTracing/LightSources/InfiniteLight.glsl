#ifndef INFINITE_LIGHT_GLSL
#define INFINITE_LIGHT_GLSL

// Uniform infinite light
const vec3 uniformSky = vec3(0.529f, 0.808f, 0.922f);
vec3 SampleInfiniteLight(Ray ray) {
    return uniformSky;
}

vec3 SampleInfiniteImageLight(const Ray ray) {
    float theta = acos(ray.direction.y); 
    float phi = atan(ray.direction.z, ray.direction.x); // azimuthal angle
    vec2 uv = vec2((phi + PI) * Inv2PI, (theta + PI / 2.0) * InvPI);
    vec3 color = texture(u_EnvLight, uv).rgb;
    return color;
}


#endif