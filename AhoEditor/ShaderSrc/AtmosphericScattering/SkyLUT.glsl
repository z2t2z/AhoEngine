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

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

const float PI = 3.14159265358979323846;
const int SAMPLE_CNT = 24;
const float Rground = 6360.0; 
const float Rtop = 6460.0;

const float SKY_LUT_WIDTH  = 192.0;
const float SKY_LUT_HEIGHT = 108.0;

uniform sampler2D u_TransmittanceLUT;
uniform sampler2D u_MultiScattLUT;

// https://www.shadertoy.com/view/tdSXzD
vec3 jodieReinhardTonemap(vec3 c){
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

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

vec3 ScatteringDirected(float h, float cosTheta) {
	vec3 RayleighCoe = vec3(5.802, 13.558, 33.1) * 1E-3;
	float RayleighScaleHeight = 8.5;

	vec3 MieCoe = vec3(3.996) * 1E-3;
	float MieScaleHeight = 1.2;

	return RayleighCoe * exp(-h / RayleighScaleHeight) * RayleighPhaseFunc(cosTheta);
		+ MieCoe * exp(-h / MieScaleHeight) * MiePhaseFunc(0.8, cosTheta);
}

vec3 ScatteringUniformPhase(float h) {
	vec3 RayleighCoe = vec3(5.802, 13.558, 33.1) * 1E-3;
	float RayleighScaleHeight = 8.5;

	vec3 MieCoe = vec3(3.996) * 1E-3;
	float MieScaleHeight = 1.2;
	
	const float uniformPhase = 1.0 / (4.0 * PI);

	return RayleighCoe * exp(-h / RayleighScaleHeight) * uniformPhase;
		+ MieCoe * exp(-h / MieScaleHeight) * uniformPhase;
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

bool Valid(vec3 P) {
	bvec3 nanCheck = isnan(P);
	return !nanCheck.x && !nanCheck.y && !nanCheck.z;
}

float fromUnitToSubUvs(float u, float resolution) { 
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f)); 
}

float fromSubUvsToUnit(float u, float resolution) { 
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f)); 
}

const float MUTIL_SCATT_RES = 32.0;

vec3 GetMultipleScattering(AtmosphereParameters Atmosphere, float viewHeight, float viewZenithCosAngle) {
	vec2 uv = vec2(viewZenithCosAngle * 0.5f + 0.5f, (viewHeight - Atmosphere.BottomRadius) / (Atmosphere.TopRadius - Atmosphere.BottomRadius));
	// uv = vec2(uv.x, 1.0 - uv.y);
	uv = clamp(uv, 0.0, 1.0);
	uv = vec2(fromUnitToSubUvs(uv.x, MUTIL_SCATT_RES), fromUnitToSubUvs(uv.y, MUTIL_SCATT_RES));

	return texture(u_MultiScattLUT, uv).rgb;
}

vec3 IntegrateScatteredLuminance(vec3 worldPos, vec3 worldDir, vec3 sunDir) {
	vec3 earthO = vec3(0.0, 0.0, 0.0);
	float tBottom = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rground);
	float tTop = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
	float tMax = 0.0f;

	// if (tBottom > 0) {
	// 	return vec3(0.0, 0.0, 0.0);
	// }

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

	if (tTop < 0) {
		return vec3(0.0, 0.0, 0.0);
	}

	float dt = tTop / SAMPLE_CNT;
	const vec3 wi = sunDir;
	const vec3 wo = worldDir;
	float cosTheta = dot(normalize(wi), normalize(wo));

	AtmosphereParameters atmosparam;
	atmosparam.TopRadius = Rtop;
	atmosparam.BottomRadius = Rground;
	
	// Ray march the atmosphere to integrate optical depth
	vec3 Lsun = vec3(25.0, 25.0, 25.0);
	// Lsun = vec3(1.0, 1.0, 1.0);
	vec3 L = vec3(0.0, 0.0, 0.0);
	const float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT);
	const float SampleSegmentT = 0.3;
	vec3 transmittance = vec3(1.0);
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
		 
		float viewHeight = length(P);

		const vec3 UpVector = P / viewHeight;
		float sunZenithCosAngle = dot(UpVector, sunDir);
		vec2 uv;
		LutTransmittanceParamsToUv(atmosparam, viewHeight, sunZenithCosAngle, uv);
		vec3 transmittanceToSun = texture(u_TransmittanceLUT, uv).rgb;

		float h = viewHeight - Rground;

		vec3 scattering = OutScattering(h);
		
		vec3 absorption = Absorption(h);

		vec3 extinction = scattering + absorption;

		vec3 sampleTransmittance = exp(-dt * extinction); 
		transmittance *= sampleTransmittance;

        vec3 ms = GetMultipleScattering(atmosparam, viewHeight, sunZenithCosAngle);
		vec3 S = Lsun
				* (ScatteringDirected(h, cosTheta) 
				* transmittanceToSun 
                + ms * scattering
				)
                ;

		vec3 Sint = (S - S * sampleTransmittance) / extinction;
		L += Sint * transmittance;
	}
	return L;
}

void UvToSkyViewLutParams(AtmosphereParameters Atmosphere, out float viewZenithCosAngle, out float lightViewCosAngle, in float viewHeight, in vec2 uv) {
	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
	uv = vec2(fromSubUvsToUnit(uv.x, 192.0f), fromSubUvsToUnit(uv.y, 108.0f));

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

uniform vec3 u_SunDir = normalize(vec3(1.0, 0.1, -0.1));

void main() {
	vec3 clipSpace = vec3(v_TexCoords * 2.0 - vec2(1.0), -1.0);
	vec4 viewPos = u_ProjectionInv * vec4(clipSpace, 1.0);
	
	if (viewPos.w != 0) {
		// viewPos /= viewPos.w;
	}
	vec3 worldDir = mat3(u_ViewInv) * viewPos.xyz; 
	worldDir = normalize(worldDir);

	vec3 worldPos = vec3(u_ViewPosition) / 1000.0;
	worldPos.y += Rground;

	float viewHeight = length(worldPos);

	float viewZenithCosAngle;
	float lightViewCosAngle;
    AtmosphereParameters atmosphere;
	UvToSkyViewLutParams(atmosphere, viewZenithCosAngle, lightViewCosAngle, viewHeight, v_TexCoords);


	vec3 sunDir;
	{
		vec3 UpVector = worldPos / viewHeight;
		float sunZenithCosAngle = dot(UpVector, u_SunDir);
		sunDir = normalize(vec3(sqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle), sunZenithCosAngle, 0.0));
	}

	worldPos = vec3(0.0f, viewHeight, 0.0f);

	float viewZenithSinAngle = sqrt(1 - viewZenithCosAngle * viewZenithCosAngle);
	worldDir = vec3(
		viewZenithSinAngle * lightViewCosAngle,
		viewZenithCosAngle,
		viewZenithSinAngle * sqrt(1.0 - lightViewCosAngle * lightViewCosAngle)
	);

    out_color = vec4(IntegrateScatteredLuminance(worldPos, worldDir, sunDir), 1.0);
}

