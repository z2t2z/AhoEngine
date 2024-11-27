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

uniform sampler2D u_TransmittanceLUT;

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

// n : sample count
vec3 GetSphericSampleDirection(int i, int j, int n) {
    float u = (float(i) + 0.5) / float(n);
    float v = (float(j) + 0.5) / float(n);

    float theta = 2.0 * PI * u;
    float phi = acos(1.0 - 2.0 * v);
    phi = clamp(phi, -1.0, 1.0);

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return vec3(x, y, z);
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

const int MS_SAMPLE_CNT = 8;

vec3 GetWorldDir(int x, int y) {
	float cnt = float(x * 8 + y);

	float i = 0.5 + float(cnt / MS_SAMPLE_CNT);
	float j = 0.5 + float(cnt - float((cnt / MS_SAMPLE_CNT) * MS_SAMPLE_CNT));

	float randA = i / MS_SAMPLE_CNT;
	float randB = j / MS_SAMPLE_CNT;
	float theta = 2.0f * PI * randA;
	float phi = acos(1.0f - 2.0f * randB);	// uniform distribution https://mathworld.wolfram.com/SpherePointPicking.html
	//phi = PI * randB;						// bad non uniform
	float cosPhi = cos(phi);
	float sinPhi = sin(phi); 
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	vec3 worldDir;
	worldDir.x = cosTheta * sinPhi;
	worldDir.y = sinTheta * sinPhi;
	worldDir.z = cosPhi;

	return worldDir;
}

vec3 RandomSphereSamples[64] = {
	vec3(-0.7838,-0.620933,0.00996137),
	vec3(0.106751,0.965982,0.235549),
	vec3(-0.215177,-0.687115,-0.693954),
	vec3(0.318002,0.0640084,-0.945927),
	vec3(0.357396,0.555673,0.750664),
	vec3(0.866397,-0.19756,0.458613),
	vec3(0.130216,0.232736,-0.963783),
	vec3(-0.00174431,0.376657,0.926351),
	vec3(0.663478,0.704806,-0.251089),
	vec3(0.0327851,0.110534,-0.993331),
	vec3(0.0561973,0.0234288,0.998145),
	vec3(0.0905264,-0.169771,0.981317),
	vec3(0.26694,0.95222,-0.148393),
	vec3(-0.812874,-0.559051,-0.163393),
	vec3(-0.323378,-0.25855,-0.910263),
	vec3(-0.1333,0.591356,-0.795317),
	vec3(0.480876,0.408711,0.775702),
	vec3(-0.332263,-0.533895,-0.777533),
	vec3(-0.0392473,-0.704457,-0.708661),
	vec3(0.427015,0.239811,0.871865),
	vec3(-0.416624,-0.563856,0.713085),
	vec3(0.12793,0.334479,-0.933679),
	vec3(-0.0343373,-0.160593,-0.986423),
	vec3(0.580614,0.0692947,0.811225),
	vec3(-0.459187,0.43944,0.772036),
	vec3(0.215474,-0.539436,-0.81399),
	vec3(-0.378969,-0.31988,-0.868366),
	vec3(-0.279978,-0.0109692,0.959944),
	vec3(0.692547,0.690058,0.210234),
	vec3(0.53227,-0.123044,-0.837585),
	vec3(-0.772313,-0.283334,-0.568555),
	vec3(-0.0311218,0.995988,-0.0838977),
	vec3(-0.366931,-0.276531,-0.888196),
	vec3(0.488778,0.367878,-0.791051),
	vec3(-0.885561,-0.453445,0.100842),
	vec3(0.71656,0.443635,0.538265),
	vec3(0.645383,-0.152576,-0.748466),
	vec3(-0.171259,0.91907,0.354939),
	vec3(-0.0031122,0.9457,0.325026),
	vec3(0.731503,0.623089,-0.276881),
	vec3(-0.91466,0.186904,0.358419),
	vec3(0.15595,0.828193,-0.538309),
	vec3(0.175396,0.584732,0.792038),
	vec3(-0.0838381,-0.943461,0.320707),
	vec3(0.305876,0.727604,0.614029),
	vec3(0.754642,-0.197903,-0.62558),
	vec3(0.217255,-0.0177771,-0.975953),
	vec3(0.140412,-0.844826,0.516287),
	vec3(-0.549042,0.574859,-0.606705),
	vec3(0.570057,0.17459,0.802841),
	vec3(-0.0330304,0.775077,0.631003),
	vec3(-0.938091,0.138937,0.317304),
	vec3(0.483197,-0.726405,-0.48873),
	vec3(0.485263,0.52926,0.695991),
	vec3(0.224189,0.742282,-0.631472),
	vec3(-0.322429,0.662214,-0.676396),
	vec3(0.625577,-0.12711,0.769738),
	vec3(-0.714032,-0.584461,-0.385439),
	vec3(-0.0652053,-0.892579,-0.446151),
	vec3(0.408421,-0.912487,0.0236566),
	vec3(0.0900381,0.319983,0.943135),
	vec3(-0.708553,0.483646,0.513847),
	vec3(0.803855,-0.0902273,0.587942),
	vec3(-0.0555802,-0.374602,-0.925519),
};

void ComputeMultiScatteringTexture(vec3 worldPos, vec3 sunDir, out vec3 L2nd_order, out vec3 f_ms) {
    vec3 earthO = vec3(0.0);
    float invSamples = 4.0 * PI / float(MS_SAMPLE_CNT * MS_SAMPLE_CNT); 

	float uniformPhase = 1.0 / (4.0 * PI);

	AtmosphereParameters atmosparam;
	atmosparam.TopRadius = Rtop;
	atmosparam.BottomRadius = Rground;

    for (int i = 0; i < MS_SAMPLE_CNT; ++i) {
        for (int j = 0; j < MS_SAMPLE_CNT; ++j) {
            vec3 rayDir = GetSphericSampleDirection(i, j, MS_SAMPLE_CNT);
			rayDir = GetWorldDir(i, j);
			// rayDir = RandomSphereSamples[i * 8 + j];

			rayDir = normalize(rayDir);

            float tAtmos = RaySphereIntersectNearest(worldPos, rayDir, earthO, Rtop);
            float tGround = RaySphereIntersectNearest(worldPos, rayDir, earthO, Rground);

            float tMax = tAtmos;
            if (tGround > 0.0) {
                tMax = tGround;
            }

            // Raymarching
            vec3 L = vec3(0.0, 0.0, 0.0);
			vec3 f_ms_sub = vec3(0.0, 0.0, 0.0);
            const float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT);
            const float SampleSegmentT = 0.3;
            vec3 transmittance = vec3(1.0);
			vec3 Lglobal = vec3(1.0);
            for (float s = 0.0f; s < SAMPLE_CNT_FLOAT; s += 1.0) {
                float t0 = s / SAMPLE_CNT_FLOAT;
                float t1 = (s + 1.0) / SAMPLE_CNT_FLOAT;

                // Non linear distribution of sample within the range.
                t0 = t0 * t0;
                t1 = t1 * t1;
                
                t0 = tMax * t0;
                t1 = t1 > 1.0 ? tMax : tMax * t1;

                float dt = abs(t1 - t0);
                float t = t0 + abs(t1 - t0) * SampleSegmentT;
                
                vec3 P = worldPos + t * rayDir; // Actual world position
                
                // vec3 transmittanceToSun = TransmittanceToSun(P, sunDir);

                float h = length(P);

				const vec3 UpVector = P / h;
				float sunZenithCosAngle = dot(UpVector, sunDir);
				vec2 uv;
				LutTransmittanceParamsToUv(atmosparam, h, sunZenithCosAngle, uv);
				vec3 transmittanceToSun = texture(u_TransmittanceLUT, uv).rgb;
				// transmittanceToSun = TransmittanceToSun(P, sunDir);

                h -= Rground;

                vec3 scattering = OutScattering(h);
                
                vec3 absorption = Absorption(h);

                vec3 extinction = scattering + absorption;

                vec3 sampleTransmittance = exp(-dt * extinction); 

				vec3 S = Lglobal
						* ScatteringUniformPhase(h) 
						* transmittanceToSun 
						;
				
				vec3 Sint = (S - S * sampleTransmittance) / extinction;
				L += Sint * transmittance;

				vec3 MSint = (scattering - scattering * sampleTransmittance) / extinction;
				f_ms_sub += MSint * transmittance;

				transmittance *= sampleTransmittance;
            }

            // TODO: Ground albedo, shadow term
            f_ms 	   += uniformPhase * f_ms_sub * invSamples;
            L2nd_order += uniformPhase * L * invSamples;
        }
    }
}

float fromUnitToSubUvs(float u, float resolution) { 
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f)); 
}

float fromSubUvsToUnit(float u, float resolution) { 
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f)); 
}

#define PLANET_RADIUS_OFFSET 0.01

void main() {
    float r;
    float mu;
    AtmosphereParameters atmosphere;

	vec2 uv = vec2(fromSubUvsToUnit(v_TexCoords.x, 32), fromSubUvsToUnit(v_TexCoords.y, 32));

	mu = uv.x * 2.0 - 1.0;
	r = Rground + clamp(uv.y + PLANET_RADIUS_OFFSET, 0.0, 1.0) * (Rtop - Rground - PLANET_RADIUS_OFFSET);

    vec3 worldPos = vec3(0.0, r, 0.0);
    vec3 sunDir = normalize(vec3(sqrt(1.0 - mu * mu), mu, 0.0));

    vec3 L2nd_order = vec3(0.0);
    vec3 f_ms = vec3(0.0);

    ComputeMultiScatteringTexture(worldPos, sunDir, L2nd_order, f_ms);

    vec3 result = L2nd_order / (1.0 - f_ms);

	out_color = vec4(result, 1.0f);
}

