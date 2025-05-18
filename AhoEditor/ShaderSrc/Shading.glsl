#type vertex
#version 460 core
#include "Common/UniformBufferObjects.glsl"

out vec2 v_TexCoords;

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

// TODO: Grid, remove this 
out vec3 v_nearP;
out vec3 v_farP;

// To world space
vec3 NDCToWorld(vec3 p) {
    vec4 pos = u_ViewInv * u_ProjectionInv * vec4(p, 1.0);
    if (pos.w != 0.0) {
        pos /= pos.w;
    }
    return pos.xyz;
}

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    v_nearP = NDCToWorld(vec3(p.xy, -1.0));
    v_farP = NDCToWorld(vec3(p.xy, 1.0));
    gl_Position = vec4(p, 1.0);
	float x = p.x, y = p.y;
	v_TexCoords = (vec2(x, y) * 0.5) + 0.5;
}

#type fragment
#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

#include "Common/UniformBufferObjects.glsl"
#include "AtmosphericScattering/AtmosphericCommon.glsl"
#include "DeferredShading/DirectLight.glsl"
#include "PathTracing/SSBO.glsl"

layout(location = 0) out vec4 out_Color;

in vec2 v_TexCoords;

// TODO: Grid, remove this 
in vec3 v_nearP;
in vec3 v_farP;

uniform sampler2D u_gDepth; 
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gAO;
uniform sampler2D u_gPBR; 

// Atmospheric 
uniform sampler2D u_SkyviewLUT;

// SSR sample counts
uniform uint u_SSRSampleCnt = 32;

// TODO: remove this
vec4 genGrid(vec3 worldPos, float scale) {
    vec2 coord = worldPos.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord); // 单位像素，coord的世界坐标的改变量
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative; // grid 的每个分量表示在屏幕空间中，当前点到最近网格线的像素距离
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    float fac = 0.5 / scale;
    if (worldPos.x > -0.5 * minimumx && worldPos.x < 0.5 * minimumx) {
        color.z = 1.0;
    }
    // x axis
    if (worldPos.z > -0.5 * minimumz && worldPos.z < 0.5 * minimumz) {
        color.x = 1.0;
    }
    return color;
}

vec4 Grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}

const float camFarPlane = 1000.0;
const float camNearPlane = 0.01;
float ComputeLinearDepth(vec3 worldPos) {
    vec4 clip_space_pos = u_Projection * u_View * vec4(worldPos.xyz, 1.0);
	return clip_space_pos.w;
}
vec4 GridColor(vec3 worldPos, float t) {
    float fromOrigin = max(0.001, abs(u_ViewPosition.y));// / camFarPlane; // [0, 1]
    vec4 s = genGrid(worldPos, 1.0) * mix(1.0, 0.0, min(1.0, fromOrigin / 100));
	// s.a = 0.0;
    vec4 m = genGrid(worldPos, 0.1) * mix(1.0, 0.0, min(1.0, fromOrigin / 200));
    vec4 l = genGrid(worldPos, 0.01) * mix(1.0, 0.0, min(1.0, fromOrigin / 300));
    vec4 GridColor = (s + m + l) * float(t > 0);
	// vec4 GridColor = (Grid(worldPos, 10, true) + Grid(worldPos, 1, true))* float(t > 0); 

	float linearDepth = ComputeLinearDepth(worldPos);
	float r = max(0.0, 1 - linearDepth / 800);
	float fading = r * r * r;
	GridColor.a *= fading;
	return GridColor;
}

// #define SKY_ATMOSPHERIC

void main() {
	vec3 fragPosVs = texture(u_gPosition, v_TexCoords).rgb; // View space
	vec3 fragPos = (u_ViewInv * vec4(fragPosVs, 1.0f)).xyz; // To world space

	float d = texture(u_gDepth, v_TexCoords).r;
	if (d == 1.0f) {
		vec2 uv = v_TexCoords;
		vec3 clipSpace = vec3(uv * 2.0 - vec2(1.0), 1.0);
		vec4 ppworldDir = u_ViewInv * u_ProjectionInv * vec4(clipSpace, 1.0);
		vec3 worldDir = normalize(vec3(ppworldDir.x, ppworldDir.y, ppworldDir.z) / ppworldDir.w);
		
#ifdef SKY_ATMOSPHERIC
		const float Rground = 6360.0; 
		vec3 worldPos = u_ViewPosition.xyz / 1000.0;
		worldPos.y = max(0.01, worldPos.y) + Rground;

		vec2 sampleUV;
		vec3 sunDir = u_DirLight[0].direction;
		SampleSkyViewLut(worldPos, worldDir, sunDir, sampleUV);
		vec3 lum = texture(u_SkyviewLUT, sampleUV).rgb;
		lum = pow(lum, vec3(1.3));
		lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0)) * 2.0 + 0.15);
		
		lum = jodieReinhardTonemap(lum);
		lum = 7 * pow(lum, vec3(1.0 / 2.2)) + GetSunLuminance(worldPos, worldDir, sunDir, Rground);
		
		float t = -v_nearP.y / (v_farP.y - v_nearP.y);
		vec3 gridWorldPos = v_nearP + t * (v_farP - v_nearP);
		
		out_Color = t < 0.0 ? vec4(lum, 1.0) : GridColor(gridWorldPos, t);
#else
		float t = -v_nearP.y / (v_farP.y - v_nearP.y);
		vec3 gridWorldPos = v_nearP + t * (v_farP - v_nearP);
		
		out_Color = t < 0.0 ? vec4(0.0, 0.0, 0.0, 1.0) : GridColor(gridWorldPos, t);
#endif
		return;
	}

	vec3 baseColor = texture(u_gAlbedo, v_TexCoords).rgb;
	out_Color = vec4(baseColor, 1.0f);
	return;
	
	float AO = texture(u_gPBR, v_TexCoords).b;
	if (AO == -1.0f) {
		AO = texture(u_gAO, v_TexCoords).r;
	}
	float metallic = texture(u_gPBR, v_TexCoords).r;
	float roughness = texture(u_gPBR, v_TexCoords).g;

	Material mat;
	mat.baseColor = baseColor;
	mat.ao = AO;
	mat.metallic = metallic;
	mat.roughness = roughness;

	vec3 viewPos = u_ViewPosition.xyz;
	vec3 N = normalize(texture(u_gNormal, v_TexCoords).rgb);
	vec3 V = normalize(viewPos - fragPos);
	vec3 F0 = mix(vec3(0.04f), mat.baseColor, mat.metallic); 
	vec3 Lo = vec3(0.1f);
	// Direct lighting
	// Lo += EvalDirectionalLight(mat, fragPos, F0, V, N);
	// Lo += EvalPointLight(mat, fragPos, F0, V, N);
	// Lo += EvalEnvLight(mat, fragPos, F0, V, N); // TODO :: WRONG

	vec3 Color = Lo / (Lo + vec3(1.0f));	// HDR tone mapping
	Color = pow(Color, vec3(1.0f / 2.2f)); 	// gamma correction

	out_Color = vec4(Color, 1.0f);
}