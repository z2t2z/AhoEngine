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
const int SAMPLE_CNT = 40;
const int SAMPLE_CNT_PRECOMPUTE = 40;
const float Rground = 6360.0; 
const float Rtop = 6460.0;
const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

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

void assert(bool condition) {
	if (!condition) {
		discard;
	}
}

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

vec3 GetAlbedo(vec3 scattering, vec3 extinction) {
	return scattering / max(vec3(0.001), extinction);
}

float GetTextureCoordFromUnitRange(float x, int texture_size) {
  return 0.5 / float(texture_size) + x * (1.0 - 1.0 / float(texture_size));
}

float GetUnitRangeFromTextureCoord(float u, int texture_size) {
  return (u - 0.5 / float(texture_size)) / (1.0 - 1.0 / float(texture_size));
}

vec2 GetTransmittanceTextureUvFromRMu(AtmosphereParameters atmosphere, float d, float r, float mu) {
  	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	// float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	float H = sqrt(Rtop * Rtop - Rground * Rground);
	// Distance to the horizon.
	float rho = sqrt(r * r - Rground * Rground);
	// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
	// float d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
	float d_min = Rtop - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
	return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
				GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

void GetRMuFromTransmittanceTextureUv(AtmosphereParameters atmosphere, vec2 uv, out float r, out float mu) {
    float x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
    float x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
	x_mu = uv.x;
	x_r = uv.y;
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
	float H = sqrt(Rtop * Rtop - Rground * Rground);
    // Distance to the horizon, from which we can compute r:
    float rho = H * x_r;
    r = sqrt(rho * rho + Rground * Rground);
    // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
    // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
    // from which we can recover mu:
    float d_min = Rtop - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    mu = d == 0.0 ? float(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
    mu = clamp(mu, -1.0, 1.0);
}

// Precompute transmittance to the top atmosphere boundary
// where mu = cosTheta 
vec3 TransmittanceToAtmosphereBoundary(float r, float mu) {
	vec3 worldDir = vec3(sqrt(1.0 - mu * mu), mu, 0.0);
	vec3 earthO = vec3(0.0, 0.0, 0.0);
    vec3 worldPos = vec3(0.0, r, 0.0);
	float tMax = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
	if (tMax < 0.0) {
		return vec3(0.0);
	}

	float dt = tMax / SAMPLE_CNT_PRECOMPUTE;
	float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT_PRECOMPUTE);
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
		assert(h > 0);
		vec3 scattering = OutScattering(h);
		vec3 absorption = Absorption(h);
		vec3 extinction = scattering + absorption;

		opticalDepth += dt * extinction;
	}

	return exp(-opticalDepth);
}

vec3 ComputeTransmittanceToTopAtmosphereBoundaryTexture(vec2 frag_coord) {
    float r;
    float mu;
    AtmosphereParameters atmosphere;
    const vec2 texSize = vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
    frag_coord /= texSize;
    GetRMuFromTransmittanceTextureUv(atmosphere, v_TexCoords, r, mu);
    
    return TransmittanceToAtmosphereBoundary(r, mu);
}

void main() {
    vec3 L = ComputeTransmittanceToTopAtmosphereBoundaryTexture(gl_FragCoord.xy);
	out_color = vec4(L, 1.0f);
}

