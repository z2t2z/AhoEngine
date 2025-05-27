#ifndef BRDF_GLSL
#define BRDF_GLSL

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI = 3.14159265358979323846;
const float TwoPI = 2 * PI;
const float InvPI = 0.31830988618379067154;
#endif

// Reference:
// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf?

// https://cdn2-unrealengine-1251447533.file.myqcloud.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf?
vec3 ImportanceSampleGGX( vec2 Xi, float Roughness, vec3 N ) {
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    vec3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 TangentX = normalize( cross( UpVector, N ) );
    vec3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float D_GGX(float roughness, float NoH) {
	float a = roughness * roughness;
	float a2 = a * a;
    NoH = max(NoH, 0.0);
	float NoH2 = NoH * NoH;
	float nom = a2;
	float denom = (NoH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return nom / denom;
}
float _G1(float cosTheta, float k) {
    float denom = cosTheta * (1.0 - k) + k;
    denom = max(denom, 1e-6);
    return cosTheta / denom;
}
float G_Smith(float NoV, float NoL, float roughness) {
    float t = (roughness + 1);
    float k = (t * t) / 8.0;
    return _G1(NoV, k) * _G1(NoL, k);
}
float SchlickWeight(float u) {
    u = clamp(1 - u, 0.0, 1.0);
    float u2 = u * u;
    return u2 * u2 * u;
}
vec3 F_Schlick(vec3 F0, float VoH) {
    float VoH2 = VoH * VoH;
    float weight = exp2(-5.55473 * VoH2 - 6.98316 * VoH); // Efficient than schlick weight according to epic
    return F0 + (vec3(1.0) - F0) * weight;
}
vec3 FresnelSchlick(vec3 F0, float cosTheta) {
	return F0 + (1.0 - F0) * SchlickWeight(cosTheta);
}
vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * SchlickWeight(cosTheta); 
}

#endif