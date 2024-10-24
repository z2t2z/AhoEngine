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

layout(location = 0) out vec4 out_Color;

in vec2 v_TexCoords;

layout(std140, binding = 1) uniform GeneralUBO{
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view

	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
	ivec4 u_Info;
};

uniform sampler2D u_DepthMap; // light's perspective
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_SSAO;
uniform sampler2D u_Specular;

uniform float u_Metalic;
uniform float u_Roughness;
uniform float u_AO;

const float PI = 3.14159265359f;

float GGX(vec3 N, vec3 H, float roughness) {
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

float FresnelSchlick(float NdotV, float roughness) {
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

float CalShadow() {
	//vec3 NDC = v_PositionLightSpace.xyz / v_PositionLightSpace.w; // [-1, 1]
	//vec3 SS = NDC * 0.5f + 0.5f; // [0, 1], ScreenSpace
	//float minDepth = texture(u_DepthMap, SS.xy).r;
	//float currDepth = SS.z;
	//return currDepth < minDepth + 0.001f ? 0.0f : 1.0f;
	return 0.0f;
}

void main() {
	float intensityScalor = 1.0f;
	float AO = texture(u_SSAO, v_TexCoords).r;
	vec3 Albedo = intensityScalor * texture(u_gAlbedo, v_TexCoords).rgb;
	Albedo = pow(Albedo, vec3(2.2f)); // To linear space
	vec3 Normal = normalize(texture(u_gNormal, v_TexCoords).rgb);
	vec3 fragPos = texture(u_gPosition, v_TexCoords).rgb;
	vec3 viewPos = vec3(u_View * u_ViewPosition);
	vec3 viewDir = normalize(viewPos - fragPos);
	out_Color = vec4(1.0f);
	vec3 F0 = vec3(0.04f); // Fresnel Schlick
	F0 = mix(F0, Albedo, u_Metalic);

	vec3 specularMap = texture(u_Specular, v_TexCoords).rgb;
	
	vec3 Lo = vec3(0.0f);
	int v_MAX_LIGHT_CNT = u_Info.x;
	for (int i = 0; i < v_MAX_LIGHT_CNT; i++) {
		vec3 lightPos = vec3(u_View * u_LightPosition[i]);
		vec3 lightDir = normalize(lightPos - fragPos);
		float NdotL = dot(Normal, lightDir);
		if (NdotL < 0.0f) {
			continue;
		}
		// Radiance per light source
		vec3 halfWay = normalize(viewDir + lightDir);
		float Distance = length(lightPos - fragPos);
		float Attenuation = 1.0f / (Distance * Distance); // inverse-square law, more physically correct
		vec3 Radiance = u_LightColor[i].rgb * (1.0f - Attenuation);
		// Cook-Torrance BRDF:
		float NDF = GGX(Normal, halfWay, u_Roughness);
		float G = GeometrySmith(Normal, viewPos, lightDir, u_Roughness);
		vec3 F = FresnelSchlick(max(dot(halfWay, viewDir), 0.0f), F0);
		vec3 kS = F;
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - u_Metalic;
		vec3 Numerator = NDF * G * F;
		float Denominator = 4.0f * max(dot(Normal, viewDir), 0.0f) * max(dot(Normal, lightDir), 0.0f) + 0.0001f;
		vec3 Specular = Numerator / Denominator;
		Lo += (kD * Albedo / PI + Specular) * Radiance * NdotL;
	}

	vec3 Ambient = vec3(0.1f) * AO * Albedo;
	vec3 Color = Ambient + Lo * (1.0f - CalShadow());
	Color = Color / (Color + vec3(1.0f));	// HDR tone mapping
	Color = pow(Color, vec3(1.0f / 2.2f)); 	// gamma correction
	//Color = mix(specularMap, Color, 0.8f);
	out_Color = vec4(Color, 1.0f);
}