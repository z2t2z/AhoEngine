#type compute
#version 460 core

#include "Common.glsl"

layout(binding = 0, rgba16f) restrict writeonly uniform imageCube outputTexture;
layout(local_size_x = 16, local_size_y = 16) in;

const uint NumSamples = 64 * 1024;
const float InvNumSamples = 1.0 / float(NumSamples);

uniform samplerCube u_gCubeMap;

vec3 SampleHemisphere(float u1, float u2) {
	float phi = 2.0f * PI * u1;
    float cosTheta = 1.0f - u2;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float z = cosTheta;
    return vec3(x, z, y);
}

void main() {
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(imageSize(outputTexture));
    uv = uv * 2.0 - 1.0;

    uint faceIdx = gl_GlobalInvocationID.z;
	vec3 N = GetSamplingVector(uv, faceIdx);
	mat3 tbn = ConstructTBN(N);
	vec3 irradiance = vec3(0);
	for (uint i = 0; i < NumSamples; ++i) {
		vec2 u  = SampleHammersley(i, InvNumSamples);
		vec3 Li = LocalToWorld(SampleHemisphere(u.x, u.y), tbn);
		float cosTheta = max(0.0, dot(Li, N));
		irradiance += 2.0 * textureLod(u_gCubeMap, Li, 0).rgb * cosTheta;
	}
	irradiance /= vec3(NumSamples);

	imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
}
