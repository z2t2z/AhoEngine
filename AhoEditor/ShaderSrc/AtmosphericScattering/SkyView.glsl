#type vertex
#version 460 core

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

out vec2 v_TexCoords;

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    gl_Position = vec4(p, 1.0);
	float x = p.x, y = p.y;
	v_TexCoords = (vec2(x, y) * 0.5) + 0.5;
}

#type fragment
#version 460 core

#include "AtmosphericCommon.glsl"
#include "../Common/UniformBufferObjects.glsl"

out vec4 out_color;
in vec2 v_TexCoords;

uniform sampler2D u_TransmittanceLUT;
uniform sampler2D u_MultiScattLUT;

float RayleighPhaseFunc(float cosTheta) {
	return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

float MiePhaseFunc(float g, float cosTheta) {
	float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
	return k * (1.0 + cosTheta * cosTheta) / pow(1.0 + g * g - 2.0 * g * cosTheta, 1.5);	
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
vec3 TransmittanceToSun(vec3 worldPos, vec3 sunDir) {
	vec3 worldDir = sunDir;  
	vec3 earthO = vec3(0.0, 0.0, 0.0);
	float tMax = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
	// tMax = RayIntersectSphere(earthO, Rtop, worldPos, worldDir); 
	if (tMax < 0.0) {
		return vec3(0.0);
	}
	float dt = tMax / SAMPLE_CNT;
	float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT);
	float SampleSegmentT = 0.3;
	vec3 L = vec3(0.0);
	for (float s = 0.0f; s < SAMPLE_CNT_FLOAT; s += 1.0) {
		float t0 = s / SAMPLE_CNT_FLOAT;
		float t1 = (s + 1.0) / SAMPLE_CNT_FLOAT;
		// Non linear distribution of sample within the range.
		t0 = t0 * t0;
		t1 = t1 * t1;
		
		t0 = tMax * t0;
		t1 = t1 > 1.0 ? tMax : tMax * t1;

		dt = t1 - t0;
		float t = t0 + dt * SampleSegmentT;

		vec3 P = worldPos + t * worldDir; // Actual world position

		float h = length(P);
		h -= Rground;

		vec3 scattering = OutScattering(h);
		vec3 absorption = Absorption(h);
		vec3 extinction = scattering + absorption;

		L += dt * extinction;
	}

	return exp(-L);
}

void LutTransmittanceParamsToUv(AtmosphereParameters Atmosphere, in float viewHeight, in float viewZenithCosAngle, out vec2 uv) {
	float H = sqrt(max(0.0f, Atmosphere.TopRadius * Atmosphere.TopRadius - Atmosphere.BottomRadius * Atmosphere.BottomRadius));
	float rho = sqrt(max(0.0f, viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius));

	float discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0) + Atmosphere.TopRadius * Atmosphere.TopRadius;
	float d = max(0.0, (-viewHeight * viewZenithCosAngle + sqrt(discriminant))); // Distance to atmosphere boundary

	float d_min = Atmosphere.TopRadius - viewHeight;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;

	uv = vec2(x_mu, x_r);
	//uv = vec2(fromUnitToSubUvs(uv.x, TRANSMITTANCE_TEXTURE_WIDTH), fromUnitToSubUvs(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT)); // No real impact so off
}

const float MUTIL_SCATT_RES = 32.0;

vec3 GetMultipleScattering(AtmosphereParameters Atmosphere, float viewHeight, float viewZenithCosAngle) {
	vec2 uv = vec2(viewZenithCosAngle * 0.5f + 0.5f, (viewHeight - Atmosphere.BottomRadius) / (Atmosphere.TopRadius - Atmosphere.BottomRadius));
	uv = vec2(fromUnitToSubUvs(uv.x, MUTIL_SCATT_RES), fromUnitToSubUvs(uv.y, MUTIL_SCATT_RES));
	uv = vec2(uv.x, 1.0 - uv.y);
	uv = clamp(uv, 0.0, 1.0);
	return texture(u_MultiScattLUT, uv).rgb;
}

vec3 IntegrateScatteredLuminance_Replace(AtmosphereParameters Atmosphere, 
										vec3 WorldPos, 
										vec3 WorldDir, 
										vec3 SunDir,
										bool MieRayPhase = true,
										bool ground = false,
										int SampleCountIni = SAMPLE_CNT,
										float tMaxMax = 9000000.0f) {
	vec3 result = vec3(0.0);
	// Compute next intersection with atmosphere or ground 
	vec3 earthO = vec3(0.0f, 0.0f, 0.0f);
	float tBottom = RaySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.BottomRadius);
	float tTop = RaySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.TopRadius);
	float tMax = 0.0f;
	if (tBottom < 0.0f) {
		if (tTop < 0.0f) {
			tMax = 0.0f; // No intersection with earth nor atmosphere: stop right away  
			return result;
		} else {
			tMax = tTop;
		}
	} else {
		if (tTop > 0.0f) {
			tMax = min(tTop, tBottom);
		}
	}

	tMax = min(tMax, tMaxMax);

	// Sample count 
	float SampleCount = SampleCountIni;
	float SampleCountFloor = SampleCountIni;
	float tMaxFloor = tMax;
	bool VariableSampleCount = false;
	// if (VariableSampleCount) {
	// 	SampleCount = lerp(RayMarchMinMaxSPP.x, RayMarchMinMaxSPP.y, saturate(tMax*0.01));
	// 	SampleCountFloor = floor(SampleCount);
	// 	tMaxFloor = tMax * SampleCountFloor / SampleCount;	// rescale tMax to map to the last entire step segment.
	// }
	float dt = tMax / SampleCount;

	// Phase functions
	const float uniformPhase = 1.0 / (4.0 * PI);
	const vec3 wi = SunDir;
	const vec3 wo = WorldDir;
	float cosTheta = dot(wi, wo);
	float MiePhaseValue = MiePhaseFunc(Atmosphere.MiePhaseG, cosTheta);
	float RayleighPhaseValue = RayleighPhaseFunc(cosTheta);

	vec3 globalL = vec3(1.0f);

	// Ray march the atmosphere to integrate optical depth
	vec3 L = vec3(0.0f);
	vec3 throughput = vec3(1.0);
	vec3 OpticalDepth = vec3(0.0);
	float t = 0.0f;
	float tPrev = 0.0;
	const float SampleSegmentT = 0.3f;
	for (float s = 0.0f; s < SampleCount; s += 1.0f) {
		if (VariableSampleCount) {
			// More expenssive but artefact free
			float t0 = (s) / SampleCountFloor;
			float t1 = (s + 1.0f) / SampleCountFloor;
			// Non linear distribution of sample within the range.
			t0 = t0 * t0;
			t1 = t1 * t1;
			// Make t0 and t1 world space distances.
			t0 = tMaxFloor * t0;
			if (t1 > 1.0) {
				t1 = tMax;
				//	t1 = tMaxFloor;	// this reveal depth slices
			} else {
				t1 = tMaxFloor * t1;
			}
			//t = t0 + (t1 - t0) * (whangHashNoise(pixPos.x, pixPos.y, gFrameId * 1920 * 1080)); // With dithering required to hide some sampling artefact relying on TAA later? This may even allow volumetric shadow?
			t = t0 + (t1 - t0)*SampleSegmentT;
			dt = t1 - t0;
		} else {
			//t = tMax * (s + SampleSegmentT) / SampleCount;
			// Exact difference, important for accuracy of multiple scattering
			float NewT = tMax * (s + SampleSegmentT) / SampleCount;
			dt = NewT - t;
			t = NewT;
		}
		vec3 P = WorldPos + t * WorldDir;

		MediumSampleRGB medium = sampleMediumRGB(P, Atmosphere);
		const vec3 SampleOpticalDepth = medium.extinction * dt;
		const vec3 SampleTransmittance = exp(-SampleOpticalDepth);
		OpticalDepth += SampleOpticalDepth;

		float pHeight = length(P);
		const vec3 UpVector = P / pHeight;
		float SunZenithCosAngle = dot(SunDir, UpVector);
		vec2 uv;
		LutTransmittanceParamsToUv(Atmosphere, pHeight, SunZenithCosAngle, uv);
		vec3 TransmittanceToSun = texture(u_TransmittanceLUT, uv).rgb;

		vec3 PhaseTimesScattering;
		if (MieRayPhase) {
			PhaseTimesScattering = medium.scatteringMie * MiePhaseValue + medium.scatteringRay * RayleighPhaseValue;
		} else {
			PhaseTimesScattering = medium.scattering * uniformPhase;
		}

		// Earth shadow 
		float tEarth = RaySphereIntersectNearest(P, SunDir, earthO + PLANET_RADIUS_OFFSET * UpVector, Atmosphere.BottomRadius);
		float earthShadow = tEarth >= 0.0f ? 0.0f : 1.0f;

		// Dual scattering for multi scattering 
		vec3 multiScatteredLuminance = vec3(0.0f);
		// TODO: Replace with LUT lookup
		multiScatteredLuminance = GetMultipleScattering(Atmosphere, pHeight, SunZenithCosAngle);

		float shadow = 1.0f;

		vec3 S = globalL * (earthShadow * shadow * TransmittanceToSun * PhaseTimesScattering + multiScatteredLuminance * medium.scattering);

		vec3 MS = medium.scattering * 1;
		vec3 MSint = (MS - MS * SampleTransmittance) / medium.extinction;
		// result.MultiScatAs1 += throughput * MSint;

		// // Evaluate input to multi scattering 
		// {
		// 	vec3 newMS;

		// 	newMS = earthShadow * TransmittanceToSun * medium.scattering * uniformPhase * 1;
		// 	result.NewMultiScatStep0Out += throughput * (newMS - newMS * SampleTransmittance) / medium.extinction;
		// 	//	result.NewMultiScatStep0Out += SampleTransmittance * throughput * newMS * dt;

		// 	newMS = medium.scattering * uniformPhase * multiScatteredLuminance;
		// 	result.NewMultiScatStep1Out += throughput * (newMS - newMS * SampleTransmittance) / medium.extinction;
		// 	//	result.NewMultiScatStep1Out += SampleTransmittance * throughput * newMS * dt;
		// }

		// See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
		vec3 Sint = (S - S * SampleTransmittance) / medium.extinction;	// integrate along the current step segment 
		L += throughput * Sint;														// accumulate and also take into account the transmittance from previous steps
		throughput *= SampleTransmittance;
		tPrev = t;
	}

	if (ground && tMax == tBottom && tBottom > 0.0) {
		// Account for bounced light off the earth
		vec3 P = WorldPos + tBottom * WorldDir;
		float pHeight = length(P);

		const vec3 UpVector = P / pHeight;
		float SunZenithCosAngle = dot(SunDir, UpVector);
		vec2 uv;
		LutTransmittanceParamsToUv(Atmosphere, pHeight, SunZenithCosAngle, uv);
		// vec3 TransmittanceToSun = TransmittanceLutTexture.SampleLevel(samplerLinearClamp, uv, 0).rgb;
		vec3 TransmittanceToSun = texture(u_TransmittanceLUT, uv).rgb;

		const float NdotL = clamp(dot(normalize(UpVector), normalize(SunDir)), 0.0, 1.0);
		L += globalL * TransmittanceToSun * throughput * NdotL * Atmosphere.GroundAlbedo / PI;
	}

	return L;	
}

void main() {
	vec2 uv = v_TexCoords;
	vec3 clipSpace = vec3(uv * 2.0 - vec2(1.0), 1.0);
	vec4 ppworldDir = u_ViewInv * u_ProjectionInv * vec4(clipSpace, 1.0);
	vec3 worldDir = normalize(vec3(ppworldDir.x, ppworldDir.y, ppworldDir.z) / ppworldDir.w);

	vec3 worldPos = u_ViewPosition.xyz / 1000.0;
	worldPos.y = max(0.01, worldPos.y) + Rground;

	float viewHeight = length(worldPos);
	float viewZenithCosAngle;
	float lightViewCosAngle;
    AtmosphereParameters atmosphere;
	UvToSkyViewLutParams(atmosphere, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);

	vec3 sunDir;
	{
		vec3 oriSunDir = u_DirLight[0].direction;
		vec3 UpVector = worldPos / viewHeight;
		float sunZenithCosAngle = dot(UpVector, oriSunDir);
		sunDir = normalize(vec3(sqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle), sunZenithCosAngle, 0.0));
	}

	worldPos = vec3(0.0f, viewHeight, 0.0f);
	float viewZenithSinAngle = sqrt(1 - viewZenithCosAngle * viewZenithCosAngle);
	worldDir = vec3(
		viewZenithSinAngle * lightViewCosAngle,
		viewZenithCosAngle,
		viewZenithSinAngle * sqrt(1.0 - lightViewCosAngle * lightViewCosAngle)
	);
	worldDir = normalize(worldDir);

	out_color = vec4(IntegrateScatteredLuminance_Replace(GetAtmosphereParameters(), worldPos, worldDir, sunDir), 1.0f);
}

