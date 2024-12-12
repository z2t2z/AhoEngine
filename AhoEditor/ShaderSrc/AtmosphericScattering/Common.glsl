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

vec3 GetSunLuminance(vec3 worldPos, vec3 worldDir, vec3 sunDir, float planetRadius) {
	// if (dot(worldDir, sunDir) > cos(0.5*0.505*3.14159 / 180.0)) {
	// 	float t = RaySphereIntersectNearest(worldPos, worldDir, vec3(0.0f, 0.0f, 0.0f), planetRadius);
	// 	if (t < 0.0f) {
	// 		const vec3 SunLuminance = 1000000.0; // arbitrary. But fine, not use when comparing the models
	// 		return SunLuminance;
	// 	}
	// }
	return vec3(0.0, 0.0, 0.0);
}

float fromUnitToSubUvs(float u, float resolution) { 
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f)); 
}

float fromSubUvsToUnit(float u, float resolution) { 
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f)); 
}

void UvToSkyViewLutParams(AtmosphereParameters Atmosphere, out float viewZenithCosAngle, out float lightViewCosAngle, in float viewHeight, in vec2 uv) {
	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
	uv = vec2(fromSubUvsToUnit(uv.x, 192.0f), fromSubUvsToUnit(uv.y, 108.0f));
	float Rground = 6360.0;
	
	float Vhorizon = sqrt(viewHeight * viewHeight - Rground * Rground);
	float CosBeta = Vhorizon / viewHeight;				// GroundToHorizonCos
	float Beta = acos(CosBeta);
	const float PI = 3.14159265358979323846;

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

void SkyViewLutParamsToUv(AtmosphereParameters Atmosphere, in bool IntersectGround, in float viewZenithCosAngle, in float lightViewCosAngle, in float viewHeight, out vec2 uv) {
	float Vhorizon = sqrt(viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius);
	float CosBeta = Vhorizon / viewHeight;				// GroundToHorizonCos
	float Beta = acos(CosBeta);
	const float PI = 3.14159265359f;
	float ZenithHorizonAngle = PI - Beta;

	if (!IntersectGround) {
		float coord = acos(viewZenithCosAngle) / ZenithHorizonAngle;
		coord = 1.0 - coord;
		coord = 1.0 - coord;
		uv.y = coord * 0.5f;
	} else {
		float coord = (acos(viewZenithCosAngle) - ZenithHorizonAngle) / Beta;
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

void SampleSkyViewLut(vec3 worldPos, vec3 worldDir, vec3 sunDir, out vec2 uv) {
	vec3 upVector = normalize(worldPos);
	float viewHeight = length(worldPos);
	float viewZenithCosAngle = dot(worldDir, upVector);

	vec3 sideVector = normalize(cross(upVector, worldDir));		// assumes non parallel vectors
	vec3 forwardVector = normalize(cross(sideVector, upVector));	// aligns toward the sun light but perpendicular to up vector
	vec2 lightOnPlane = vec2(dot(sunDir, forwardVector), dot(sunDir, sideVector));
	lightOnPlane = normalize(lightOnPlane);
	float lightViewCosAngle = lightOnPlane.x;

	AtmosphereParameters Atmosphere;
	Atmosphere.BottomRadius = 6360.0f;
	bool intersectGround = RaySphereIntersectNearest(worldPos, worldDir, vec3(0.0, 0.0, 0.0), Atmosphere.BottomRadius) >= 0.0f;
	SkyViewLutParamsToUv(Atmosphere, false, viewZenithCosAngle, lightViewCosAngle, viewHeight, uv);
	// uv.y = 1.0 - uv.y;
	// return texture(skyViewLut, uv).rgb;
}
