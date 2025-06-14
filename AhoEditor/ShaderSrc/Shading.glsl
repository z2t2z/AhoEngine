#type vertex
#version 460 core
#include "Common/UniformBufferObjects.glsl"

const vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

// TODO: Grid, remove this 
out vec3 v_nearP;
out vec3 v_farP;

// To world space
vec3 NDCToWorld(vec3 p, mat4 viewInv, mat4 projInv) {
    vec4 pos = viewInv * projInv * vec4(p, 1.0);
    if (pos.w != 0.0) {
        pos /= pos.w;
    }
    return pos.xyz;
}

void main() {
    vec3 p = gridPlane[gl_VertexID].xyz;
    v_nearP = NDCToWorld(vec3(p.xy, -1.0), u_ViewInv, u_ProjectionInv);
    v_farP = NDCToWorld(vec3(p.xy, 1.0), u_ViewInv, u_ProjectionInv);
    gl_Position = vec4(p, 1.0);
}

#type fragment
#version 460 core

#include "Common/UniformBufferObjects.glsl"
#include "AtmosphericScattering/AtmosphericCommon.glsl"
#include "DeferredShading/DirectLight.glsl"

layout(location = 0) out vec4 out_Color;

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

void main() {
	vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(u_gAlbedo, 0));
	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec3 vs_Pos = texelFetch(u_gPosition, coord, 0).xyz; // view space
	vec3 fragPos = (u_ViewInv * vec4(vs_Pos, 1)).xyz; // world space

	float d = texelFetch(u_gDepth, coord, 0).r;
	if (d == 1.0f) {
		vec3 clipSpace = vec3(uv * 2.0 - vec2(1.0), 1.0);
		vec4 ppworldDir = u_ViewInv * u_ProjectionInv * vec4(clipSpace, 1.0);
		vec3 worldDir = normalize(vec3(ppworldDir.x, ppworldDir.y, ppworldDir.z) / ppworldDir.w);

#ifdef FEATURE_ENABLE_IBL				
		vec3 cubemap = texture(u_gCubeMap, worldDir).rgb;
		cubemap = pow(cubemap, vec3(1.0 / 2.2f)); // gamma correction
		out_Color = vec4(cubemap, 1.0f);
		return;
#endif

		float t = -v_nearP.y / (v_farP.y - v_nearP.y);
		vec3 gridWorldPos = v_nearP + t * (v_farP - v_nearP);
#ifdef FEATURE_ENABLE_SKYATMOSPHERIC
		const float Rground = 6360.0; 
		vec3 worldPos = u_ViewPosition.xyz / 1000.0;
		worldPos.y = max(0.01, worldPos.y) + Rground;
		vec2 sampleUV;
		vec3 sunDir = u_SunDir;
		SampleSkyViewLut(worldPos, worldDir, sunDir, sampleUV);
		vec3 lum = texture(u_SkyviewLUT, sampleUV).rgb;
		// lum = pow(lum, vec3(1.3));
		lum /= (smoothstep(0.0, 0.2, clamp(sunDir.y, 0.0, 1.0)) * 2.0 + 0.15);
		lum = jodieReinhardTonemap(lum);
		lum = pow(lum, vec3(1.0 / 2.2)) + GetSunLuminance(worldPos, worldDir, sunDir, Rground);
		out_Color = t < 0.0 ? vec4(lum, 1.0) : GridColor(gridWorldPos, t);
		return;
#endif

		out_Color = t < 0.0 ? vec4(0.0, 0.0, 0.0, 1.0) : GridColor(gridWorldPos, t);
		return;
	}

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

	vec3 viewPos = u_ViewPosition.xyz; // in view space
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