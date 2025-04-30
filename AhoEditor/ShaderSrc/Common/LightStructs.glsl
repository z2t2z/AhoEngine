#ifndef LIGHT_STRUCT_GLSL
#define LIGHT_STRUCT_GLSL

struct PointLight {
	vec4 position; // world space, radius in w
	vec4 color;    // RGB, intensity in W
};

struct DirectionalLight {
	vec3 direction; // world space
	float _padding;
	vec3 color;     
	float intensity;
	mat4 lightProjView; // light projection view matrix
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