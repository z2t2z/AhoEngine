#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core

#include "AtmosphericScattering/Common.glsl"
#include "UniformBufferObjects.glsl"

layout(location = 0) out vec4 out_Color;

in vec2 v_TexCoords;

uniform sampler2D u_gLightDepth; // light's perspective
uniform sampler2D u_gDepth; 
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gAO;
uniform sampler2D u_gPBR; 

// IBL
uniform samplerCube u_gIrradiance;
uniform samplerCube u_gPrefilter;
uniform sampler2D u_gLUT;

// Atmospheric 
uniform sampler2D u_SkyviewLUT;

// SSR sample counts
uniform uint u_SSRSampleCnt = 32;

const float PI = 3.14159265359f;
const float MAX_REFLECTION_LOD = 4.0;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// SSR screen space ray tracing
bool IsHit(out vec2 hitUV, vec3 startPos, vec3 endPos) {
	// Clip space
    // vec4 H0 = ViewSpaceToClipSpace(beginPos);
    // vec4 H1 = ViewSpaceToClipSpace(endPos);
    // float k0 = 1.0f / H0.w;
    // float k1 = 1.0f / H1.w;

	return false;

}

// Shadow
vec2 poissonDisk[16] = {
	vec2( -0.94201624, -0.39906216 ),
	vec2( 0.94558609, -0.76890725 ),
	vec2( -0.094184101, -0.92938870 ),
	vec2( 0.34495938, 0.29387760 ),
	vec2( -0.91588581, 0.45771432 ),
	vec2( -0.81544232, -0.87912464 ),
	vec2( -0.38277543, 0.27676845 ),
	vec2( 0.97484398, 0.75648379 ),
	vec2( 0.44323325, -0.97511554 ),
	vec2( 0.53742981, -0.47373420 ),
	vec2( -0.26496911, -0.41893023 ),
	vec2( 0.79197514, 0.19090188 ),
	vec2( -0.24188840, 0.99706507 ),
	vec2( -0.81409955, 0.91437590 ),
	vec2( 0.19984126, 0.78641367 ),
	vec2( 0.14383161, -0.14100790 )
}; 

const float nearPlane = 0.1f; 
const float frustumWidth = 100.0f;
const float lightSize = 2.0f; 	// light width
const float lightUV = lightSize / frustumWidth;
const int SAMPLE_CNT = 16;
float BIAS = 0.0001f;
vec2 depthMapDt;
float zPosFromLight;

float FindBlocker(vec2 uv, float zReceiver) {
	float searchWidth = lightUV * (zPosFromLight - nearPlane) / zPosFromLight;

	float blockerSum = 0.0f;
	int sampleCnt = 0;

	for (int i = 0; i < SAMPLE_CNT; ++i) {
		float sampleDepth = texture(u_gLightDepth, uv + poissonDisk[i] * searchWidth).r;
		if (zReceiver > sampleDepth) {
			blockerSum += sampleDepth;
			sampleCnt += 1;
		}
	}

	if (sampleCnt == 0) {
		return -1.0f;
	}

	return blockerSum / float(sampleCnt);
}

float PCF(vec2 uv, float zReceiver, float radius, float bias) {
	float shadowSum = 0.0f;

	for (int i = 0; i < SAMPLE_CNT; ++i) {
		float sampleDepth = texture(u_gLightDepth, uv + poissonDisk[i] * radius).r;
		shadowSum += zReceiver > sampleDepth + 0.0025f? 0.0f : 1.0f;
	}

	return shadowSum / float(SAMPLE_CNT);
}

float PCSS(vec4 fragPos, float NdotL, int lightIdx) {
	if (lightIdx != 0) {
		return 1.0f;
	}
	fragPos.w = 1.0f;
	fragPos = u_LightPV[lightIdx] * fragPos; 	// To light space 

	fragPos /= fragPos.w;			
	fragPos = fragPos * 0.5f + 0.5f;
	zPosFromLight = fragPos.z;
 
	vec2 uv = fragPos.xy;
	float zReceiver = fragPos.z;

	float bias = max(0.05 * (1.0 - NdotL), 0.005);

	float avgBlockerDepth = FindBlocker(uv, zReceiver);
	if (avgBlockerDepth == -1.0f) {
		return 1.0f;
	}

	float Wpenumbra = (zReceiver - avgBlockerDepth) * lightUV / avgBlockerDepth;

	float pcfRadius = Wpenumbra * nearPlane / zReceiver;

	if (fragPos.z > 1.0f) {
		return 1.0f;
	}
	return PCF(uv, zReceiver, pcfRadius, bias);
}


void main() {
	vec3 fragPosVs = texture(u_gPosition, v_TexCoords).rgb; // View space
	vec3 fragPos = (u_ViewInv * vec4(fragPosVs, 1.0f)).xyz; // To world space

	float d = texture(u_gDepth, v_TexCoords).r;
	if (d == 1.0f) { 
		vec3 clipSpace = vec3(v_TexCoords * 2.0 - vec2(1.0), -1.0);
		vec4 viewPos = u_ProjectionInv * vec4(clipSpace, 1.0);
		
		if (viewPos.w != 0) {
			viewPos /= viewPos.w;
		}
		vec3 worldDir = mat3(u_ViewInv) * viewPos.xyz; 
		worldDir = normalize(worldDir);

		const float Rground = 6360.0; 
		vec3 worldPos = vec3(u_ViewPosition);
		worldPos /= 1000.0f;
		worldPos.y += Rground;

		vec3 sunDir = normalize(vec3(0.0, 0.001, 0.5));
		vec2 sampleUV;
		SampleSkyViewLut(worldPos, worldDir, sunDir, sampleUV);
		vec3 lum = texture(u_SkyviewLUT, sampleUV).rgb;
		lum = pow(lum, vec3(1.3));
		lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0)) * 2.0 + 0.15);
		
		lum = jodieReinhardTonemap(lum);
		
		lum = pow(lum, vec3(1.0 / 2.2));
		out_Color = vec4(lum, 1.0);
		return;
	}

	vec3 albedo = pow(texture(u_gAlbedo, v_TexCoords).rgb, vec3(2.2f)); 	// To linear space
	float AO = texture(u_gPBR, v_TexCoords).b;
	if (AO == -1.0f) {
		AO = texture(u_gAO, v_TexCoords).r;
	}
	float metalic = texture(u_gPBR, v_TexCoords).r;
	float roughness = texture(u_gPBR, v_TexCoords).g;

	vec3 viewPos = u_ViewPosition.xyz;
	vec3 N = normalize(texture(u_gNormal, v_TexCoords).rgb);
	vec3 V = normalize(viewPos - fragPos); 		// View direction
	vec3 R = reflect(-V, N);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metalic); 	// Metalic workflow
	
	vec3 Lo = vec3(0.0f);
	// Direct lighting
	for (int i = 0; i < MAX_LIGHT_CNT; ++i) {
		if (u_Info[i].x == 0) {
			break;
		}

		vec3 lightPos = vec3(u_LightPosition[i]);
		vec3 L = normalize(lightPos - fragPos);		// Light direction
		float NdotL = dot(N, L);

		if (NdotL < 0.0f) {
			continue;
		}

		vec3 H = normalize(V + L);
		float distance = length(lightPos - fragPos);
		float attenuation = 1.0f / (distance * distance); 	// Inverse-square law, more physically correct
		vec3 radiance = u_LightColor[i].rgb * (1.0f - attenuation);
		
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

		vec3 Numerator = NDF * G * F;
		float Denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
	
		vec3 kS = F;
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - metalic;

		vec3 specular = Numerator / Denominator;
		Lo += (kD * albedo / PI + specular) 
					* radiance 
					* NdotL 
					* PCSS(vec4(fragPos, 1.0f), NdotL, i);
	}

	// Indirect lighting
	// for (int i = 0; i < u_SSRSampleCnt; ++i) {

	// }

	// Ambient lighting
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metalic;

	vec3 irradiance = texture(u_gIrradiance, N).rgb;
	vec3 diffuse = irradiance * albedo;

	vec3 preFilter = textureLod(u_gPrefilter, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(u_gLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = preFilter * (F * brdf.x + brdf.y);
	
	vec3 ambient = (kD * diffuse + specular) * AO;

	vec3 Color = ambient + Lo;
	Color = Color / (Color + vec3(1.0f));	// HDR tone mapping
	Color = pow(Color, vec3(1.0f / 2.2f)); 	// gamma correction

	out_Color = vec4(Color, 1.0f);
}