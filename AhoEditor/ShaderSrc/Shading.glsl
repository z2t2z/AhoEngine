#type vertex
#version 460 core
#include "Common/UniformBufferObjects.glsl"

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    gl_Position = vec4(p, 1.0);
}

#type fragment
#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#define ACES
#define FEATURE_RAY_TRACE_SHADOW

layout(location = 0) out vec4 out_Color;

#include "Common/UniformBufferObjects.glsl"
#include "AtmosphericScattering/AtmosphericCommon.glsl"
#include "DeferredShading/DirectLight.glsl"
#include "DeferredShading/Indirect.glsl"
#include "./DDGI/DDGICommon.glsl"

// G-buffers
uniform sampler2D u_gDepth; 
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gAO;
uniform sampler2D u_gPBR; 

uniform DDGIVolumeDescGPU u_DDGIVolumeDesc;

// --- Simple post processing ---
// Reinhard Tone Mapping
vec3 ToneMap_Reinhard(vec3 color) {
    return color / (color + vec3(1.0));
}
vec3 ToneMap_ACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}
vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}
vec3 EncodeSRGB(vec3 color) {
    vec3 lt = color * 12.92;
    vec3 gt = pow(color, vec3(1.0 / 2.4)) * 1.055 - 0.055;
    return mix(lt, gt, step(0.0031308, color));
}
vec3 ACES_ToneMap(vec3 color) {
    const float exposure = 1.0; 
    color *= exposure / 0.6; // normalize HDR 亮度以适配 ACES
    color = RRTAndODTFit(color);
    return clamp(color, 0.0, 1.0);
}
// Gamma Correction
vec3 GammaCorrect(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
	vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(u_gAlbedo, 0));
	ivec2 coord = ivec2(gl_FragCoord.xy);
	float d = texelFetch(u_gDepth, coord, 0).r;

	vec3 Color;
	if (d == 1.0f) {
		// Get background radiance 
		Color.rgb = EvalBackground(uv, u_ViewPosition.xyz, u_ViewInv, u_ProjectionInv).rgb;
	} else {
		vec3 vs_Pos = texelFetch(u_gPosition, coord, 0).xyz; // view space
		vec4 fragPost = u_ViewInv * vec4(vs_Pos, 1);
		vec3 fragPos = fragPost.xyz / fragPost.w; // world space

		vec3 baseColor = texelFetch(u_gAlbedo, coord, 0).rgb;
		// baseColor = pow(baseColor, vec3(2.2f)); // gamma correction

		float metallic = texelFetch(u_gPBR, coord, 0).r;
		float roughness = texelFetch(u_gPBR, coord, 0).g;
		float AO = texelFetch(u_gPBR, coord, 0).b;

		Material mat;
		mat.baseColor = baseColor;
		mat.ao = AO;
		mat.metallic = metallic;
		mat.roughness = roughness;

		vec3 viewPos = u_ViewPosition.xyz; // in world space
		vec3 vs_N = normalize(texelFetch(u_gNormal, coord, 0).rgb); // view space normal
		vec3 N = transpose(inverse(mat3(u_ViewInv))) * vs_N; // world space
		N = normalize(N);
		vec3 V = normalize(viewPos - fragPos);
		vec3 F0 = mix(vec3(0.04f), mat.baseColor, mat.metallic); 
		vec3 Lo = vec3(0.0f);
		// Direct lighting
		// Lo += EvalDirectionalLight(mat, fragPos, F0, V, N);
		Lo += EvaluateDirectionalLight(mat, fragPos, N, V);
		// Indirect lighting
		vec3 indirect = GetIndirectLighting(u_DDGIVolumeDesc, u_ViewPosition.xyz, fragPos, N, mat.baseColor, mat.ao);
		Lo += indirect;
		
	#ifdef FEATURE_ENABLE_IBL
		Lo += EvalEnvLight(mat, fragPos, F0, V, N);
	#endif
		Color = Lo;
	}

#ifdef ACES
	vec3 toneMapped = ToneMap_ACES(Color);
	// vec3 toneMapped = ACES_ToneMap(Color);
#else
	vec3 toneMapped = ToneMap_Reinhard(Color);
#endif

	vec3 finalColor = toneMapped;
	finalColor = GammaCorrect(toneMapped);
	// finalColor = EncodeSRGB(toneMapped);

	out_Color = vec4(finalColor, 1.0f);
}