#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140) uniform CameraData{
	mat4 u_View;
	mat4 u_Projection;
	mat4 u_Model0; // deprecated!
	vec4 u_ViewPosition;
	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
};

out vec3 v_Position;
out vec2 v_TexCoords;
out vec3 v_ViewPosition;
out vec3 v_LightPosition[4];
out vec3 v_LightColor[4];

uniform mat4 u_Model;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0f);
	v_Position = vec3(u_Model * vec4(a_Position, 1.0f));
	v_TexCoords = a_TexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
	vec3 T = normalize(normalMatrix * a_Tangent);
	vec3 N = normalize(normalMatrix * a_Normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	for (int i = 0; i < 4; i++) {
		vec3 lightPos = vec3(u_LightPosition[i].r, u_LightPosition[i].g, u_LightPosition[i].b);
		v_LightPosition[i] = TBN * lightPos;
		v_LightColor[i] = vec3(u_LightColor[i].r, u_LightColor[i].g, u_LightColor[i].b);
	}
	v_ViewPosition = TBN * vec3(u_ViewPosition);
	v_Position = TBN * v_Position;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 out_EntityColor;
layout(location = 1) out vec4 out_Color;

in vec3 v_Position;
in vec2 v_TexCoords;
in vec3 v_ViewPosition;
in vec3 v_LightPosition[4];
in vec3 v_LightColor[4];

uniform uint u_EntityID;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;

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

void main() {
	float intensityScalor = 1.0f;
	vec3 Albedo = intensityScalor * texture(u_Diffuse, v_TexCoords).rgb;
	vec3 Normal = normalize(texture(u_Normal, v_TexCoords).rgb);
	vec3 viewDir = normalize(v_ViewPosition - v_Position);

	vec3 F0 = vec3(0.04f); // Fresnel Schlick
	F0 = mix(F0, Albedo, u_Metalic);

	vec3 Lo = vec3(0.0f);
	for (int i = 0; i < 4; i++) {
		// Radiance per light source
		vec3 lightDir = normalize(v_LightPosition[i] - v_Position);
		vec3 halfWay = normalize(lightDir + viewDir);
		float Distance = length(v_LightPosition[i] - v_Position);
		float Attenuation = 1.0f / (Distance * Distance); // inverse-square law, more physically correct
		vec3 Radiance = v_LightColor[i] * Attenuation;
		// Cook-Torrance BRDF:
		float NDF = GGX(Normal, halfWay, u_Roughness);
		float G = GeometrySmith(Normal, v_ViewPosition, lightDir, u_Roughness);
		vec3 F = FresnelSchlick(max(dot(halfWay, viewDir), 0.0), F0);
		vec3 kS = F;
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - u_Metalic;
		vec3 Numerator = NDF * G * F;
		float Denominator = 4.0f * max(dot(Normal, viewDir), 0.0f) * max(dot(Normal, lightDir), 0.0f) + 0.0001f;
		vec3 Specular = Numerator / Denominator;
		float NdotL = max(dot(Normal, lightDir), 0.0f);
		Lo += (kD * Albedo / PI + Specular) * Radiance * NdotL;
	}

	vec3 Ambient = vec3(0.03f) * Albedo * (1.0f - u_AO);
	vec3 Color = Ambient + Lo;
	Color = Color / (Color + vec3(1.0f));
	Color = pow(Color, vec3(1.0f / 2.2f)); // gamma correction
	out_Color = vec4(Color, 1.0f);

	out_EntityColor = vec4(
		float(u_EntityID & 0xFF) / 255.0f,              // R  
		float((u_EntityID >> 8) & 0xFF) / 255.0f,       // G
		float((u_EntityID >> 16) & 0xFF) / 255.0f,      // B 
		float((u_EntityID >> 24) & 0xFF) / 255.0f       // A
	);
}