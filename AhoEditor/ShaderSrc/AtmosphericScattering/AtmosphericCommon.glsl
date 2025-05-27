#ifndef ATMOSPHERIC_COMMON_GLSL
#define ATMOSPHERIC_COMMON_GLSL

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI = 3.14159265358979323846;
const float TwoPI = 2 * PI;
const float InvPI = 0.31830988618379067154;
#endif

const int SAMPLE_CNT = 24;
const int SAMPLE_CNT_TRANSMITTANCE = 40;

const float Rground = 6360.0; 
const float Rtop = 6460.0;

const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

const float SKY_LUT_WIDTH  = 192.0;
const float SKY_LUT_HEIGHT = 108.0;

#define PLANET_RADIUS_OFFSET 0.01f

// https://www.shadertoy.com/view/tdSXzD
vec3 jodieReinhardTonemap(vec3 c){
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

struct AtmosphereParameters {
	// Radius of the planet (center to ground)
	float BottomRadius;
	// Maximum considered atmosphere height (center to atmosphere top)
	float TopRadius;

	// Rayleigh scattering exponential distribution scale in the atmosphere
	float RayleighDensityExpScale;
	// Rayleigh scattering coefficients
	vec3 RayleighScattering;

	// Mie scattering exponential distribution scale in the atmosphere
	float MieDensityExpScale;
	// Mie scattering coefficients
	vec3 MieScattering;
	// Mie extinction coefficients
	vec3 MieExtinction;
	// Mie absorption coefficients
	vec3 MieAbsorption;
	// Mie phase function excentricity
	float MiePhaseG;

	// An atmosphere layer of width 'width', and whose density is defined as
	// 'ExpTerm' * exp('ExpScale' * h) + 'LinearTerm' * h + 'ConstantTerm',
	// clamped to [0,1], and where h is the altitude.	
	// Refer to Bruneton's implementation of definitions.glsl for more details
	// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/183ead5bdacc701b3b626347a680a2f3cd3d4fbd/Resources/Bruneton17/definitions.glsl
	float Width0;
	float ExpTerm0;
	float ExpScale0;
	float LinearTerm0;
	float ConstantTerm0;

	float Width1;
	float ExpTerm1;
	float ExpScale1;
	float LinearTerm1;
	float ConstantTerm1;

	// Another medium type in the atmosphere
	float AbsorptionDensity0LayerWidth;
	float AbsorptionDensity0ConstantTerm;
	float AbsorptionDensity0LinearTerm;
	float AbsorptionDensity1ConstantTerm;
	float AbsorptionDensity1LinearTerm;
	// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
	vec3 AbsorptionExtinction;
	// The albedo of the ground.
	vec3 GroundAlbedo;
};
AtmosphereParameters GetAtmosphereParameters() {
	AtmosphereParameters Parameters;
	Parameters.AbsorptionExtinction = vec3(0.65, 1.881, 0.085) * 1E-3;

	// Traslation from Bruneton2017 parameterisation.
	Parameters.RayleighDensityExpScale = -1.0 / 8.0;
	Parameters.MieDensityExpScale = -1.0 / 1.2;
	Parameters.AbsorptionDensity0LayerWidth = 25.0f;
	Parameters.AbsorptionDensity0ConstantTerm = -0.666666687;
	Parameters.AbsorptionDensity0LinearTerm = 0.066666666667;
	Parameters.AbsorptionDensity1ConstantTerm = 2.66666675;
	Parameters.AbsorptionDensity1LinearTerm = -0.0666666701;

	Parameters.MiePhaseG = 0.8;
	Parameters.RayleighScattering = vec3(5.802f, 13.558f, 33.1f) * 1E-3f;
	Parameters.MieScattering = vec3(0.003996f, 0.003996f, 0.003996f);
	Parameters.MieAbsorption = vec3(0.004440f, 0.004440f, 0.004440f);
	Parameters.MieExtinction = vec3(0.004440f, 0.004440f, 0.004440f);
	Parameters.GroundAlbedo = vec3(1.0);
	Parameters.BottomRadius = 6360.0f;
	Parameters.TopRadius = 6460.0f;

	// ==============================Reference=======================================
	// Parameters.RayleighDensityExpScale = rayleigh_density[1].w;
	// Parameters.MieDensityExpScale = mie_density[1].w;
	// Parameters.AbsorptionDensity0LayerWidth = absorption_density[0].x;
	// Parameters.AbsorptionDensity0ConstantTerm = absorption_density[1].x;
	// Parameters.AbsorptionDensity0LinearTerm = absorption_density[0].w;
	// Parameters.AbsorptionDensity1ConstantTerm = absorption_density[2].y;
	// Parameters.AbsorptionDensity1LinearTerm = absorption_density[2].x;

	// Parameters.MiePhaseG = mie_phase_function_g;
	// Parameters.RayleighScattering = rayleigh_scattering;
	// Parameters.MieScattering = mie_scattering;
	// Parameters.MieAbsorption = mie_absorption;
	// Parameters.MieExtinction = mie_extinction;
	// Parameters.GroundAlbedo = ground_albedo;
	// Parameters.BottomRadius = bottom_radius;
	// Parameters.TopRadius = top_radius;	
	return Parameters;
}

struct MediumSampleRGB {
	vec3 scattering;
	vec3 absorption;
	vec3 extinction;

	vec3 scatteringMie;
	vec3 absorptionMie;
	vec3 extinctionMie;

	vec3 scatteringRay;
	vec3 absorptionRay;
	vec3 extinctionRay;

	vec3 scatteringOzo;
	vec3 absorptionOzo;
	vec3 extinctionOzo;

	vec3 albedo;
};
float getAlbedo(float scattering, float extinction) {
	return scattering / max(0.001, extinction);
}
vec3 getAlbedo(vec3 scattering, vec3 extinction) {
	return scattering / max(vec3(0.001), extinction);
}
MediumSampleRGB sampleMediumRGB(in vec3 WorldPos, in AtmosphereParameters Atmosphere) {
	const float viewHeight = length(WorldPos) - Atmosphere.BottomRadius;

	const float densityMie = exp(Atmosphere.MieDensityExpScale * viewHeight);
	const float densityRay = exp(Atmosphere.RayleighDensityExpScale * viewHeight);
	const float densityOzo = clamp(viewHeight < Atmosphere.AbsorptionDensity0LayerWidth ?
		Atmosphere.AbsorptionDensity0LinearTerm * viewHeight + Atmosphere.AbsorptionDensity0ConstantTerm :
		Atmosphere.AbsorptionDensity1LinearTerm * viewHeight + Atmosphere.AbsorptionDensity1ConstantTerm, 0.0, 1.0);

	MediumSampleRGB s;

	s.scatteringMie = densityMie * Atmosphere.MieScattering;
	s.absorptionMie = densityMie * Atmosphere.MieAbsorption;
	s.extinctionMie = densityMie * Atmosphere.MieExtinction;

	s.scatteringRay = densityRay * Atmosphere.RayleighScattering;
	s.absorptionRay = vec3(0.0);
	s.extinctionRay = s.scatteringRay + s.absorptionRay;

	s.scatteringOzo = vec3(0.0);
	s.absorptionOzo = densityOzo * Atmosphere.AbsorptionExtinction;
	s.extinctionOzo = s.scatteringOzo + s.absorptionOzo;

	s.scattering = s.scatteringMie + s.scatteringRay + s.scatteringOzo;
	s.absorption = s.absorptionMie + s.absorptionRay + s.absorptionOzo;
	s.extinction = s.extinctionMie + s.extinctionRay + s.extinctionOzo;
	s.albedo = getAlbedo(s.scattering, s.extinction);

	return s;
}

uniform AtmosphereParameters u_AtmosParams;
uniform vec3 u_SunDir;

// Used in fragment shader only
void assert(bool condition) {
	if (!condition) {
		discard;
	}
}

// - r0: ray origin
// - rd: normalized ray direction
// - s0: sphere center
// - sR: sphere radius
// - Returns the distance(solution) of the first intersecion from r0 to sphere,
//   or -1.0 if no intersection.
float RaySphereIntersectNearest(vec3 r0, vec3 rd, vec3 s0, float sR) {
	float a = dot(rd, rd);
	vec3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sR * sR);
	float delta = b * b - 4.0 * a * c;
	if (delta < 0.0 || a == 0.0) {
		return -1.0;
	}
	float sol0 = (-b - sqrt(delta)) / (2.0 * a);
	float sol1 = (-b + sqrt(delta)) / (2.0 * a);
	if (sol0 < 0.0 && sol1 < 0.0) {
		// assert(false);
		return -1.0;
	}
	if (sol0 < 0.0) {
		return max(0.0, sol1);
	} else if (sol1 < 0.0) {
		return max(0.0, sol0);
	}
	return max(0.0, min(sol0, sol1));
}

float fromUnitToSubUvs(float u, float resolution) { 
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f)); 
}

float fromSubUvsToUnit(float u, float resolution) { 
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f)); 
}

// From sebh's 
void UvToSkyViewLutParams(AtmosphereParameters Atmosphere, out float viewZenithCosAngle, out float lightViewCosAngle, in float viewHeight, vec2 uv) {
	uv = vec2(fromSubUvsToUnit(uv.x, 192.0f), fromSubUvsToUnit(uv.y, 108.0f));
	float Rground = 6360.0;
	
	float Vhorizon = sqrt(viewHeight * viewHeight - Rground * Rground);
	float CosBeta = Vhorizon / viewHeight;				// GroundToHorizonCos
	float Beta = acos(CosBeta);
	float ZenithHorizonAngle = PI - Beta;

	if (uv.y < 0.5f) {
		float coord = 2.0 * uv.y;
		coord = 1.0 - coord;
		coord *= coord;
		coord = 1.0 - coord;
		viewZenithCosAngle = cos(ZenithHorizonAngle * coord);
	} else {
		float coord = uv.y * 2.0 - 1.0;
		coord *= coord;
		viewZenithCosAngle = cos(ZenithHorizonAngle + Beta * coord);
	}

	float coord = uv.x;
	coord *= coord;
	lightViewCosAngle = -(coord * 2.0 - 1.0);
}

void SkyViewLutParamsToUv(AtmosphereParameters Atmosphere, in bool IntersectGround, in float viewZenithCosAngle, in float lightViewCosAngle, in float viewHeight, inout vec2 uv) {
	float Vhorizon = sqrt(viewHeight * viewHeight - Rground * Rground);
	float CosBeta = Vhorizon / viewHeight;				// GroundToHorizonCos
	float Beta = acos(CosBeta);
	float ZenithHorizonAngle = PI - Beta;

	if (!IntersectGround) {
		float coord = acos(viewZenithCosAngle) / ZenithHorizonAngle;
		coord = 1.0 - coord;
		coord = sqrt(coord);
		coord = 1.0 - coord;
		uv.y = coord * 0.5f;
	} else {
		float coord = (acos(viewZenithCosAngle) - ZenithHorizonAngle) / Beta;
		coord = sqrt(coord);
		uv.y = coord * 0.5f + 0.5f;
	}

	{
		float coord = -lightViewCosAngle * 0.5f + 0.5f;
		coord = sqrt(coord);
		uv.x = coord;
	}

	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
	uv = vec2(fromUnitToSubUvs(uv.x, 192.0f), fromUnitToSubUvs(uv.y, 108.0f));
}

// from sebh's
void UvToLutTransmittanceParams(AtmosphereParameters Atmosphere, out float viewHeight, out float viewZenithCosAngle, in vec2 uv) {
	float x_mu = uv.x;
	float x_r = uv.y;

	// TODO
	{
		Atmosphere.TopRadius = Rtop;
		Atmosphere.BottomRadius = Rground;
	}
	
	float H = sqrt(Atmosphere.TopRadius * Atmosphere.TopRadius - Atmosphere.BottomRadius * Atmosphere.BottomRadius);
	float rho = H * x_r;
	viewHeight = sqrt(rho * rho + Atmosphere.BottomRadius * Atmosphere.BottomRadius);

	float d_min = Atmosphere.TopRadius - viewHeight;
	float d_max = rho + H;
	float d = d_min + x_mu * (d_max - d_min);
	viewZenithCosAngle = d == 0.0 ? 1.0f : (H * H - rho * rho - d * d) / (2.0 * viewHeight * d);
	viewZenithCosAngle = clamp(viewZenithCosAngle, -1.0, 1.0);
}

void SampleSkyViewLut(vec3 worldPos, vec3 worldDir, vec3 sunDir, inout vec2 uv) {
	AtmosphereParameters Atmosphere;
	Atmosphere.BottomRadius = Rground;
	float viewHeight = length(worldPos);
	vec3 upVector = normalize(worldPos);
	float viewZenithCosAngle = dot(worldDir, upVector);

	vec3 sideVector = normalize(cross(upVector, worldDir));		// assumes non parallel vectors
	vec3 forwardVector = normalize(cross(sideVector, upVector));	// aligns toward the sun light but perpendicular to up vector
	vec2 lightOnPlane = vec2(dot(sunDir, forwardVector), dot(sunDir, sideVector));
	lightOnPlane = normalize(lightOnPlane);
	float lightViewCosAngle = lightOnPlane.x;
	bool intersectGround = RaySphereIntersectNearest(worldPos, worldDir, vec3(0.0, 0.0, 0.0), Atmosphere.BottomRadius) >= 0.0f;
	SkyViewLutParamsToUv(Atmosphere, intersectGround, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);
}

vec3 GetSunLuminance(vec3 WorldPos, vec3 WorldDir, vec3 sun_direction, float PlanetRadius) {
	if (dot(WorldDir, sun_direction) > cos(0.5*0.505*3.14159 / 180.0)) {
		float t = RaySphereIntersectNearest(WorldPos, WorldDir, vec3(0.0f, 0.0f, 0.0f), PlanetRadius);
		if (t < 0.0f) {
			const vec3 SunLuminance = vec3(1000000.0); // arbitrary. But fine, not use when comparing the models
			return SunLuminance;
		}
	}
	return vec3(0.0, 0.0, 0.0);
}

#endif