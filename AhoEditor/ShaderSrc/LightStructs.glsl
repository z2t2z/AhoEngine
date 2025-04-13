#ifndef LIGHT_STRUCT_GLSL
#define LIGHT_STRUCT_GLSL

// Note: should be aligned with C++ code

struct PointLight {
	vec4 position; // world space, radius in w
	vec4 color;    // RGB, intensity in W
};

struct DirectionalLight {
	vec4 direction; // world space
	vec4 color;     // RGB, intensity in w
};

struct SpotLight {
	vec4 position; // world space, radius in w
	vec4 direction; // world space
	vec4 color;     // RGB, intensity in w
	float innerAngle; // cos(theta)
	float outerAngle; // cos(phi)
	float range;      // distance to far plane
	float falloff;    // 1.0 / (1.0 + a * d + b * d^2)
};

#endif