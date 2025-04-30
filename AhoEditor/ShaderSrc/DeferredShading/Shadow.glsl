#ifndef SHADOW_GLSL
#define SHADOW_GLSL

uniform sampler2D u_gLightDepth; // light's perspective

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
const int PCSS_SAMPLE_CNT = 16;
float BIAS = 0.0001f;
float zPosFromLight;

float FindBlocker(vec2 uv, float zReceiver) {
	float searchWidth = lightUV * (zPosFromLight - nearPlane) / zPosFromLight;

	float blockerSum = 0.0f;
	int sampleCnt = 0;

	for (int i = 0; i < PCSS_SAMPLE_CNT; ++i) {
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
	for (int i = 0; i < PCSS_SAMPLE_CNT; ++i) {
		float sampleDepth = texture(u_gLightDepth, uv + poissonDisk[i] * radius).r;
		shadowSum += zReceiver > sampleDepth + 0.0025f? 0.0f : 1.0f;
	}
	return shadowSum / float(PCSS_SAMPLE_CNT);
}

float PCSS(vec4 fragPos, float NdotL, mat4 lightPV) {
	fragPos.w = 1.0f;
	fragPos = lightPV * fragPos; 	// To light space 

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

#endif