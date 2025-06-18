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

#include "Common/UniformBufferObjects.glsl"
#include "AtmosphericScattering/AtmosphericCommon.glsl"
#include "DeferredShading/DirectLight.glsl"

layout(location = 0) out vec4 out_Color;

// G-buffers
uniform sampler2D u_gDepth; 
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gAO;
uniform sampler2D u_gPBR; 

void main() {
	vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(u_gAlbedo, 0));
	ivec2 coord = ivec2(gl_FragCoord.xy);
	float d = texelFetch(u_gDepth, coord, 0).r;
	// Get background radiance 
	if (d == 1.0f) {
		out_Color = EvalBackground(uv, u_ViewPosition.xyz, u_ViewInv, u_ProjectionInv);
		return;
	}

	vec3 vs_Pos = texelFetch(u_gPosition, coord, 0).xyz; // view space
	vec3 fragPos = (u_ViewInv * vec4(vs_Pos, 1)).xyz; // world space

	vec3 baseColor = texelFetch(u_gAlbedo, coord, 0).rgb;
	baseColor = pow(baseColor, vec3(2.2f)); // gamma correction

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
	vec3 V = normalize(viewPos - fragPos);
	vec3 F0 = mix(vec3(0.04f), mat.baseColor, mat.metallic); 
	vec3 Lo = vec3(0.0f);
	// Direct lighting
	Lo += EvalDirectionalLight(mat, fragPos, F0, V, N);
	// Lo += EvalPointLight(mat, fragPos, F0, V, N);

#ifdef FEATURE_ENABLE_IBL
	Lo += EvalEnvLight(mat, fragPos, F0, V, N);
#endif

	vec3 Color = Lo / (Lo + vec3(1.0f));	// HDR tone mapping
	Color = pow(Color, vec3(1.0f / 2.2f)); 	// gamma correction

	// TODO: ACES tone mapping
	out_Color = vec4(Color, 1.0f);
}