#ifndef INDIRECT_LIGHTING_GLSL
#define INDIRECT_LIGHTING_GLSL

#include "BRDF.glsl"
#include "../DDGI/DDGIHelpers.glsl"

vec3 GetIndirectLighting(const DDGIVolumeDescGPU volume, vec3 camPos, vec3 worldPosition, vec3 worldNormal, vec3 diffuseColor, float ambientOcclusion = 1.0) {
	vec3 indirectLighting = vec3(0);

	int ddgiVolumesIdx = 0; // TODO, put this in a uniform buffer
	if (ddgiVolumesIdx >= 0) {
		vec3 Wo = normalize(camPos - worldPosition);
		indirectLighting = DiffuseBRDF(diffuseColor) * SampleDDGIIrradiance(volume, worldPosition, worldNormal, -Wo);
	} else {
		// TODO: Handle case when no DDGI volumes
		const vec3 ambientColor = vec3(0.1f, 0.1f, 0.1f);
		indirectLighting = 0.1f * ambientColor * diffuseColor * ambientOcclusion;
	}
	return indirectLighting;
}


#endif