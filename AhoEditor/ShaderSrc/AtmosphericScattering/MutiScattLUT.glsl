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

#include "Common.glsl"

out vec4 out_color;
in vec2 v_TexCoords;

uniform sampler2D u_TransmittanceLUT;

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
	worldDir.y = cosPhi;
	worldDir.z = sinTheta * sinPhi;

	return worldDir;
}

void ComputeMultiScatteringTexture(vec3 worldPos, vec3 sunDir, out vec3 L2nd_order, out vec3 f_ms) {
    vec3 earthO = vec3(0.0);
    float invSamples = 4.0 * PI / float(MS_SAMPLE_CNT * MS_SAMPLE_CNT); 
	float uniformPhase = 1.0 / (4.0 * PI);
	AtmosphereParameters atmosparam;
	atmosparam.TopRadius = Rtop;
	atmosparam.BottomRadius = Rground;

    for (int i = 0; i < MS_SAMPLE_CNT; ++i) {
        for (int j = 0; j < MS_SAMPLE_CNT; ++j) {
			vec3 rayDir = GetWorldDir(i, j);
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
			vec3 Lglobal = vec3(1.0);	// hardcode
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

void main() {
    float r;
    float mu;
    AtmosphereParameters atmosphere;

	vec2 uv = vec2(fromSubUvsToUnit(v_TexCoords.x, 32), fromSubUvsToUnit(v_TexCoords.y, 32));

	mu = uv.x * 2.0 - 1.0;
	r = Rground + clamp(uv.y + PLANET_RADIUS_OFFSET, 0.0, 1.0) * (Rtop - Rground - PLANET_RADIUS_OFFSET);

    vec3 worldPos = vec3(0.0, r, 0.0);
	vec3 sunDir = normalize(vec3(0.0, sqrt(1.0 - mu * mu), mu));

    vec3 L2nd_order = vec3(0.0);
    vec3 f_ms = vec3(0.0);

    ComputeMultiScatteringTexture(worldPos, sunDir, L2nd_order, f_ms);

    vec3 result = L2nd_order / (1.0 - f_ms);

	out_color = vec4(result, 1.0f);
}

