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

uniform vec3 u_SunDir = normalize(vec3(1.0, 0.1, -0.1));

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

float RayIntersectSphere(vec3 center, float radius, vec3 rayStart, vec3 rayDir) {
    float OS = length(center - rayStart);
    float SH = dot(center - rayStart, rayDir);
    float OH = sqrt(OS*OS - SH * SH);
    float PH = sqrt(radius*radius - OH * OH);

    // ray miss sphere
    if(OH > radius) return -1;

    // use min distance
    float t1 = SH - PH;
    float t2 = SH + PH;
    float t = (t1 < 0) ? t2 : t1;

    return t;
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
	// return vec3(0.0);
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

vec3 TransmittanceToSun(vec3 worldPos) {
	vec3 worldDir = u_SunDir;  
	
	vec3 earthO = vec3(0.0, 0.0, 0.0);
	float tMax = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
	// tMax = RayIntersectSphere(earthO, Rtop, worldPos, worldDir); 
	if (tMax < 0.0) {
		return vec3(0.0);
	}

	float dt = tMax / SAMPLE_CNT;
	float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT);
	float SampleSegmentT = 0.5;
	vec3 L = vec3(0.0);
	for (float s = 0.0f; s < SAMPLE_CNT_FLOAT; s += 1.0) {
		float t0 = s / SAMPLE_CNT_FLOAT;
		float t1 = (s + 1.0) / SAMPLE_CNT_FLOAT;
		// Non linear distribution of sample within the range.
		t0 = t0 * t0;
		t1 = t1 * t1;
		
		t0 = tMax * t0;
		t1 = t1 > 1.0 ? tMax : tMax * t1;

		float t = t0 + (t1 - t0) * SampleSegmentT;
		t = (t0 + t1) / 2.0;
		dt = t1 - t0;

		vec3 P = worldPos + t * worldDir; // Actual world position

		vec3 dis0 = worldPos + t0 * worldDir;
		vec3 dis1 = worldPos + t1 * worldDir;
		dt = length(dis0 - dis1);
		

		float h = length(P);
		h -= Rground;
		assert(h > 0);
		vec3 scattering = OutScattering(h);
		vec3 absorption = Absorption(h);
		vec3 extinction = scattering + absorption;


		L += dt * extinction;
	}

	return exp(-L);
}

bool Valid(vec3 P) {
	bvec3 nanCheck = isnan(P);
	return !nanCheck.x && !nanCheck.y && !nanCheck.z;
}

vec3 BruteForceRaymarching() {
	vec3 clipSpace = vec3(v_TexCoords * 2.0 - vec2(1.0), -1.0);
	vec4 viewPos = u_ProjectionInv * vec4(clipSpace, 1.0);
	
	if (viewPos.w != 0) {
		viewPos /= viewPos.w;
	}
	vec3 worldDir = mat3(u_ViewInv) * viewPos.xyz; 
	worldDir = normalize(worldDir);

	// worldDir = normalize(vec3(u_ViewInv * (viewPos / viewPos.w)));

	vec3 worldPos = vec3(u_ViewPosition) / 1000.0;
	worldPos.y += Rground;

	vec3 earthO = vec3(0.0, 0.0, 0.0);

	float tBottom = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rground);
	float tTop = RaySphereIntersectNearest(worldPos, worldDir, earthO, Rtop);
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

	if (tTop < 0) {
		return vec3(0.0, 0.0, 0.0);
	}

	float dt = tTop / SAMPLE_CNT;
	const vec3 wi = u_SunDir;
	const vec3 wo = worldDir;
	float cosTheta = dot(normalize(wi), normalize(wo));

	// Ray march the atmosphere to integrate optical depth
	vec3 Lsun = vec3(1.0, 1.0, 1.0);
	vec3 L = vec3(0.0, 0.0, 0.0);
	const float SAMPLE_CNT_FLOAT = float(SAMPLE_CNT);
	const float SampleSegmentT = 0.5;
	vec3 opticalDepth = vec3(0.0, 0.0, 0.0);
	for (float s = 0.0f; s < SAMPLE_CNT_FLOAT; s += 1.0) {
		float t0 = s / SAMPLE_CNT_FLOAT;
		float t1 = (s + 1.0) / SAMPLE_CNT_FLOAT;

		// Non linear distribution of sample within the range.
		t0 = t0 * t0;
		t1 = t1 * t1;
		
		t0 = tMax * t0;
		t1 = t1 > 1.0 ? tMax : tMax * t1;

		dt = abs(t1 - t0);
		float t = t0 + abs(t1 - t0) * SampleSegmentT;
		
		vec3 P = worldPos + t * worldDir; // Actual world position
		
		vec3 transmittanceToSun = TransmittanceToSun(P);
		float h = length(P);
		h -= Rground;
		assert(h > 0);
		vec3 scattering = OutScattering(h);
		
		vec3 absorption = Absorption(h);

		vec3 extinction = scattering + absorption;

		opticalDepth += dt * extinction;

		vec3 inscattering = vec3(25.0)
							* ScatteringDirected(h, cosTheta) 
							* exp(-opticalDepth) 
							* dt 
							* transmittanceToSun 
							* Lsun;
		L += inscattering;
	}
	return L;
}

void main() {
    vec3 L = BruteForceRaymarching();
	out_color = vec4(L, 1.0f);
}

