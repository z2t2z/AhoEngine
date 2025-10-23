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
const float frustumWidth = 20.0f;
const float lightSize = 10.0f; 	// light width
const float lightRadius = 10.0f;
const float lightUV = lightSize / frustumWidth;
const int PCSS_SAMPLE_CNT = 16;
float BIAS = 0.0001f;
float zPosFromLight;

const int PCSS_NUM_BLOCKERS = 16;

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

vec3 GetLightSpaceCoords(vec4 fragPosWS, mat4 lightPV) {
	vec4 fragPosLS = lightPV * fragPosWS;  // light space
	fragPosLS /= fragPosLS.w;
	vec3 projCoords = fragPosLS.xyz * 0.5 + 0.5;
	return projCoords;
}

float SimpleHardShadow(vec3 projCoords, float LoN) {
    // float recordedDepth = texture(u_gLightDepth, projCoords.xy).r;
	float recordedDepth = texelFetch(u_gLightDepth, ivec2(projCoords.xy * textureSize(u_gLightDepth, 0)), 0).r;
    float actualDepth   = projCoords.z;
    // const float k = 0.001;
	// float bias = min(k * LoN, 0.005);  // No need for bias if using backface culling
    return actualDepth > recordedDepth ? 0.0 : 1.0;
}

float SoftShadow_PCF(vec3 projCoords, float radius = 1.5) {
    float actualDepth   = projCoords.z;
	const vec2 poissonDisk[4] = vec2[](
		vec2( -0.94201624, -0.39906216 ),
		vec2( 0.94558609, -0.76890725 ),
		vec2( -0.094184101, -0.92938870 ),
		vec2( 0.34495938, 0.29387760 )
	);

	float shadow = 0.0f;
	vec2 texSize = textureSize(u_gLightDepth, 0);
	for (int i = 0; i < 4; ++i) {
#ifdef TEXELFETCH_PCF
		ivec2 xy = ivec2((projCoords.xy + poissonDisk[i] / texSize) * texSize);
		float recordedDepth = texelFetch(u_gLightDepth, xy, 0).r;
#else
		vec2 offset = poissonDisk[i] / texSize * radius;
		float recordedDepth = texture(u_gLightDepth, projCoords.xy + offset).r;
#endif
		shadow += actualDepth > recordedDepth ? 0.0f : 1.0f;
	}
	return shadow / 4.0f;
}

float SoftShadow_PCSS(vec3 projCoords) {
	vec2 uv = projCoords.xy;
	float zReceiver = projCoords.z;

	// --- Blocker Search ---
	// float searchRadius = lightUV * (zReceiver - nearPlane) / zReceiver;
	float searchRadius = lightRadius * (zReceiver - nearPlane) / zReceiver;

	int blockerCount = 0;
	float avgBlockerDepth = 0.0f;
	vec2 texSize = textureSize(u_gLightDepth, 0);
	for (int i = 0; i < PCSS_NUM_BLOCKERS; ++i) {
		vec2 offset = poissonDisk[i] / texSize * searchRadius;
        float recordedDepth = texture(u_gLightDepth, uv + offset).r;
        if (recordedDepth < zReceiver) {
            avgBlockerDepth += recordedDepth;
            blockerCount++;
        }
	}
	if (blockerCount == 0)
		return 1.0f; // No shadow

	avgBlockerDepth /= float(blockerCount);

	// --- Penumbra Size ---
    float penumbraRadius = lightSize * (zReceiver - avgBlockerDepth) / avgBlockerDepth;

	// --- PCF ---
	return SoftShadow_PCF(projCoords, penumbraRadius);
}

float GetShadowAttenuation(vec4 fragPosWS, float LoN, mat4 lightPV) {
	vec3 lightSpaceCoords = GetLightSpaceCoords(fragPosWS, lightPV);
	// early exit if outside shadow map
	if (lightSpaceCoords.x < 0.0 || lightSpaceCoords.x > 1.0 ||
		lightSpaceCoords.y < 0.0 || lightSpaceCoords.y > 1.0)
		return 1.0; 
		
	// return SimpleHardShadow(lightSpaceCoords, LoN);
	// return SoftShadow_PCF(lightSpaceCoords);
	return SoftShadow_PCSS(lightSpaceCoords);
	return 1.0f;
}


#endif