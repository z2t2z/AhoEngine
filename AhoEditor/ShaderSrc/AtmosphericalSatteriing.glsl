#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position.xy, -1.0f, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_color;
in vec2 v_TexCoords;

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

const float PI = 3.14159265358979323846;
const int SAMPLE_CNT = 24;
const float Rground = 6360.0 * 1000; 
const float Rtop = 6460.0 * 1000;

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
	vec3 AbsorptionExtinction;
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


	// The albedo of the ground.
	vec3 GroundAlbedo;
};

uniform AtmosphereParameters u_AtmosParams;

AtmosphereParameters gAtmosParams;

void InitializeAtmosphereParams() {
	// Scale in km
	gAtmosParams.BottomRadius 				= 6360.0;
	gAtmosParams.TopRadius 					= 6460.0;
	gAtmosParams.RayleighDensityExpScale 	= 8.0;	
	gAtmosParams.MieDensityExpScale			= 1.2;	// Also called 'scale height'  

	gAtmosParams.RayleighScattering 		= vec3(0.000650f, 0.001881f, 0.000085f);
	
	gAtmosParams.MieScattering 				= vec3(0.003996f, 0.003996f, 0.003996f);
	gAtmosParams.MieExtinction				= vec3(0.004440f, 0.004440f, 0.004440f);

	gAtmosParams.GroundAlbedo = vec3(0.0);

	// Ozone https://github.com/ebruneton/precomputed_atmospheric_scattering/blob/master/atmosphere/demo/demo.cc#L246
	// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
	// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
	// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
	// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
	gAtmosParams.Width0 		= 25.0;
	gAtmosParams.ExpTerm0 		= 0.0;
	gAtmosParams.ExpScale0 		= 0.0;
	gAtmosParams.LinearTerm0 	= 1.0 / 15.0;
	gAtmosParams.ConstantTerm0 	= -2.0 / 3.0;

	gAtmosParams.Width1 		= 0.0;
	gAtmosParams.ExpTerm1 		= 0.0;
	gAtmosParams.ExpScale1 		= 0.0;
	gAtmosParams.LinearTerm1 	= -1.0 / 15.0;
	gAtmosParams.ConstantTerm1 	= 8.0 / 3.0;

	gAtmosParams.AbsorptionExtinction = vec3(0.000650f, 0.001881f, 0.000085f);
}

vec3 GetFragWorldPos() {
    vec4 pos = vec4(v_TexCoords.x, v_TexCoords.y, 0.0f, 1.0f);
    pos = pos * 2.0 - 1.0;
    pos = u_ViewInv * pos;
	return vec3(pos);
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
		return -1.0;
	}
	if (sol0 < 0.0) {
		return max(0.0, sol1);
	} else if (sol1 < 0.0) {
		return max(0.0, sol0);
	}
	return max(0.0, min(sol0, sol1));
}

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
	/* Rayleigh does not have absorption */
	float mieScaleHeight = 1.2 * 1000;
	float mieDensity = exp(-h / mieScaleHeight);
	vec3 mieAbsorption(4.40); 

	float ozoWidth = 30.0 * 1000;
	float ozoCenterAltitude = 25.0 * 1000;
	float ozoDensity = max(0.0, 1.0 - abs(h - ozoCenterAltitude) / ozoWidth);
	vec3 ozoAbsorption(0.65, 1.881, 0.085);

	return mieAbsorption * mieDensity + ozoAbsorption * ozoDensity;
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

float saturate(float v) {
	return clamp(v, 0.0, 1.0);
}

float getAlbedo(float scattering, float extinction) {
	return scattering / max(0.001, extinction);
}

vec3 getAlbedo(vec3 scattering, vec3 extinction) {
	return scattering / max(0.001, extinction);
}

// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/master/Resources/RenderSkyCommon.hlsl#L231
MediumSampleRGB SampleMediumRGB(vec3 worldPos) {
	MediumSampleRGB s;

	const float viewHeight = length(worldPos) - Rground;

	const float densityMie = exp(gAtmosParams.MieDensityExpScale * viewHeight);
	const float densityRay = exp(gAtmosParams.RayleighDensityExpScale * viewHeight);
	const float densityOzo = saturate(viewHeight > gAtmosParams.Width0 ?
		gAtmosParams.LinearTerm0 * viewHeight + gAtmosParams.ConstantTerm0 :
		gAtmosParams.LinearTerm1 * viewHeight + gAtmosParams.ConstantTerm1);

	s.scatteringMie = densityMie * gAtmosParams.MieScattering;
	s.absorptionMie = densityMie * gAtmosParams.MieAbsorption;
	s.extinctionMie = densityMie * gAtmosParams.MieExtinction;

	s.scatteringRay = densityRay * gAtmosParams.RayleighScattering;
	s.absorptionRay = 0.0;
	s.extinctionRay = s.scatteringRay + s.absorptionRay;

	s.scatteringOzo = 0.0;
	s.absorptionOzo = densityOzo * gAtmosParams.AbsorptionExtinction;
	s.extinctionOzo = s.scatteringOzo + s.absorptionOzo;

	s.scattering = s.scatteringMie + s.scatteringRay + s.scatteringOzo;
	s.absorption = s.absorptionMie + s.absorptionRay + s.absorptionOzo;
	s.extinction = s.extinctionMie + s.extinctionRay + s.extinctionOzo;
	s.albedo = GetAlbedo(s.scattering, s.extinction);

	return s;
}

uniform vec3 u_SunDir;

vec3 TransmittanceToSun(vec3 worldPos) {
	vec3 worldDir = u_SunDir;
	vec3 earthO(0.0);
	float tBottom = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rground);
	float tTop = RaySphereIntersectNearest(worldPos, worldDir, earth, Rtop);
	float tMax = 0.0f;
	if (tBottom < 0.0f) {
		if (tTop < 0.0f) {
			tMax = 0.0f; // No intersection with earth nor atmosphere: stop right away  
			return vec3(0.0);
		} else {
			tMax = tTop; 
		}
	} else {
		if (tTop > 0.0f) {
			tMax = min(tTop, tBottom);
		}
	}

	float dt = tMax / SAMPLE_CNT;
	const vec3 wi = u_SunDir;
	const vec3 wo = worldDir;

	float RayleighPhaseVal = RayleighPhaseFunc(cosTheta);
	// vec3 RayleighScatteredCoe = vec3(5.802, 13.558, 33.1) * 
	float RayleighInscattered = RayleighScatteredCoe * RayleighPhaseVal; 
	gAtmosParams.RayleighScattering = RayleighInscattered;

	float MiePhaseVal = MiePhaseFunc(0.8, cosTheta);
	float MieScatteredCoe;
	float MieInscattered = MieScatteredCoe * MiePhase;


	// Ray march the atmosphere to integrate optical depth
	vec3 Lsun;
	vec3 L(0.0);
	const float SAMPLE_CNT_F = float(SAMPLE_CNT);
	const float SampleSegmentT = 0.3;
	vec3 opticalDepth(0.0);
	for (float s = 0.0f; s < SAMPLE_CNT_F; s += 1.0) {
		float t0 = s / SAMPLE_CNT_F;
		float t1 = (s + 1.0) / SAMPLE_CNT_F;
		// Non linear distribution of sample within the range.
		t0 = t0 * t0;
		t1 = t1 * t1;
		
		t0 = tMax * t0;
		t1 = t1 > 1.0 ? tMax : tMax * t1;

		t = t0 + (t1 - t0) * SampleSegmentT;
		dt = t1 - t0;

		vec3 P = worldPos + t * worldDir; // Actual world position
		MediumSampleRGB sampleResult = SampleMediumRGB(P);		
		vec3 extinction = sampleResult.extinction;
		L += extinction * dt;
	}

	return exp(-L);
}

vec3 BruteForceRaymarching() {
	vec3 clipSpace = vec3(v_TexCoords * 2.0 - vec2(1.0), 1.0);
	vec4 viewPos = u_ProjectionInv * vec4(clipSpace, 1.0);
	
	vec3 worldDir = mat3(u_ViewInv) * (viewPos.xyz / viewPos.w);  
	vec3 worldPos = vec3(u_ViewPosition) + Rground;
	
	vec3 earthO(0.0);
	float tBottom = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rground);
	float tTop = RaySphereIntersectNearest(worldPos, worldDir, earth, Rtop);
	float tMax = 0.0f;
	if (tBottom < 0.0f) {
		if (tTop < 0.0f) {
			tMax = 0.0f; // No intersection with earth nor atmosphere: stop right away  
			return vec3(0.0);
		} else {
			tMax = tTop; 
		}
	} else {
		if (tTop > 0.0f) {
			tMax = min(tTop, tBottom);
		}
	}

	float dt = tMax / SAMPLE_CNT;
	const vec3 wi = u_SunDir;
	const vec3 wo = worldDir;
	float cosTheta = dot(wi, wo);

	float RayleighPhaseVal = RayleighPhaseFunc(cosTheta);
	// vec3 RayleighScatteredCoe = vec3(5.802, 13.558, 33.1) * 
	float RayleighInscattered = RayleighScatteredCoe * RayleighPhaseVal; 
	gAtmosParams.RayleighScattering = RayleighInscattered;

	float MiePhaseVal = MiePhaseFunc(0.8, cosTheta);
	float MieScatteredCoe;
	float MieInscattered = MieScatteredCoe * MiePhase;


	// Ray march the atmosphere to integrate optical depth
	vec3 Lsun;
	vec3 L(0.0);
	const float SAMPLE_CNT_F = float(SAMPLE_CNT);
	const float SampleSegmentT = 0.3;
	vec3 opticalDepth(0.0);
	for (float s = 0.0f; s < SAMPLE_CNT_F; s += 1.0) {
		float t0 = s / SAMPLE_CNT_F;
		float t1 = (s + 1.0) / SAMPLE_CNT_F;
		// Non linear distribution of sample within the range.
		t0 = t0 * t0;
		t1 = t1 * t1;
		
		t0 = tMax * t0;
		t1 = t1 > 1.0 ? tMax : tMax * t1;

		t = t0 + (t1 - t0) * SampleSegmentT;
		dt = t1 - t0;

		vec3 P = worldPos + t * worldDir; // Actual world position



		vec3 TransmittanceToSun;
		// vec3 Transmittance = exp(-)

		vec3 PhaseTimesScattering = 

		L += Lsun * (MieInscattered + RayleighInscattered) * T0 * T1;
	}

	return L;
}

void main() {
	InitializeAtmosphereParams();

	out_color = vec4(1.0f);
}

