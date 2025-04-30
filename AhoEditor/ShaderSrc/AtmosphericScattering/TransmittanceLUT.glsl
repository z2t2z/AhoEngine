#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position.xy, -1.0f, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_color;
in vec2 v_TexCoords;

#include "AtmosphericCommon.glsl"
#include "../Common/UniformBufferObjects.glsl"

float RayleighPhaseFunc(float cosTheta) {
	return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

// g is a asymmetry parameter ranging from [-1, 1], by default 0.8
float MiePhaseFunc(float g, float cosTheta) {
	float coe = 3.0 / (8.0 * PI);
	float numer = (1.0 - g * g) * (1.0 + cosTheta * cosTheta);
	float tmp = (1.0 + g * g - 2.0 * g * cosTheta);
	float denom = (2.0 + g * g) * tmp * sqrt(tmp); // faster than pow(x, 1.5)?
	return coe * numer / denom;
}

float HGPhaseFunc(float g, float cosTheta) {
	// http://www.pbr-book.org/3ed-2018/Volume_Scattering/Phase_Functions.html
	float numer = 1.0f - g * g;
	float denom = 1.0f + g * g + 2.0f * g * cosTheta;
	return numer / (4.0f * PI * denom * sqrt(denom));
}  

// From Table 1, "A Scalable and Production Ready Sky and gAtmosParams Rendering Technique"
vec3 Absorption(float h) {
	// Rayleigh does not have absorption 
	float mieScaleHeight = 1.2;
	float mieDensity = exp(-h / mieScaleHeight);
	vec3 mieAbsorption = vec3(4.40, 4.40, 4.40) * 1E-3; 

	float ozoWidth = 30.0;
	float ozoCenterAltitude = 25.0;
	float ozoDensity = max(0.0, 1.0 - abs(h - ozoCenterAltitude) / ozoWidth);
	vec3 ozoAbsorption = vec3(0.65, 1.881, 0.085) * 1E-3;

	return mieAbsorption * mieDensity + ozoAbsorption * ozoDensity;
}

vec3 OutScattering(float h) {
	// Rayleigh and Mie scattering
	vec3 RayleighCoe = vec3(5.802, 13.558, 33.1) * 1E-3;
	float RayleighScaleHeight = 8.5;

	vec3 MieCoe = vec3(3.996) * 1E-3;
	float MieScaleHeight = 1.2;

	return RayleighCoe * exp(-h / RayleighScaleHeight) + MieCoe * exp(-h / MieScaleHeight);
}

// Indenpendent of sun direction
vec3 IntegrateTransmittanceToAtmosphereBoundary(AtmosphereParameters atmos, vec3 worldPos, vec3 worldDir) {
	vec3 earthO = vec3(0.0, 0.0, 0.0);
	float tMax = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
	if (tMax < 0.0) {
		return vec3(0.0);
	}
	float dt = tMax / SAMPLE_CNT_TRANSMITTANCE;
	float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT_TRANSMITTANCE);
	float SampleSegmentT = 0.3;
	vec3 opticalDepth = vec3(0.0);
	float t = 0.0;
	for (float s = 0.0f; s < SAMPLE_CNT_FLOAT; s += 1.0) {
		float NewT = tMax * (s + SampleSegmentT) / SAMPLE_CNT_FLOAT;
		dt = NewT - t;
		t = NewT;
		vec3 P = worldPos + t * worldDir; // Actual world position
		float h = length(P);
		h -= Rground;
		vec3 scattering = OutScattering(h);
		vec3 absorption = Absorption(h);
		vec3 extinction = scattering + absorption;
		opticalDepth += dt * extinction;
	}

	return exp(-opticalDepth);
}

vec4 RenderTransmittanceLUT() {
	float viewHeight;
	float viewZenithCosAngle;
	AtmosphereParameters atmosphere;
	UvToLutTransmittanceParams(atmosphere, viewHeight, viewZenithCosAngle, v_TexCoords);

	vec3 worldPos = vec3(0.0, viewHeight, 0.0);
	vec3 worldDir = vec3(0.0, viewZenithCosAngle, sqrt(1.0 - viewZenithCosAngle * viewZenithCosAngle));

	vec3 transmittance = IntegrateTransmittanceToAtmosphereBoundary(atmosphere, worldPos, worldDir);
	return vec4(transmittance, 1.0);
}

void main() {
	out_color = RenderTransmittanceLUT();
}

