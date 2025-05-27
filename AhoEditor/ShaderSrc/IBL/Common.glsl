#ifndef IBL_COMMON_GLSL
#define IBL_COMMON_GLSL

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI = 3.14159265358979323846;
const float TwoPI = 6.28318530717958647692;
const float InvPI = 0.31830988618379067154;
#endif

const float Epsilon = 0.00001;

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 SampleHammersley(uint i, float InvNumSamples) {
	return vec2(float(i) * InvNumSamples, RadicalInverse_VdC(i));
}
vec3 GetSamplingVector(vec2 uv, uint faceIdx) {
    vec3 ret;
    if (faceIdx == 0)      ret = vec3(1.0,  -uv.y, -uv.x);
    else if (faceIdx == 1) ret = vec3(-1.0, -uv.y,  uv.x);
    else if (faceIdx == 2) ret = vec3(uv.x, 1.0, uv.y);
    else if (faceIdx == 3) ret = vec3(uv.x, -1.0, -uv.y);
    else if (faceIdx == 4) ret = vec3(uv.x, -uv.y, 1.0);
    else if (faceIdx == 5) ret = vec3(-uv.x, -uv.y, -1.0);
    return normalize(ret);
}
void ComputeBasisVectors(const vec3 N, out vec3 S, out vec3 T) {
	// Branchless select non-degenerate T.
	T = cross(N, vec3(0.0, 1.0, 0.0));
	T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));
	T = normalize(T);
	S = normalize(cross(N, T));
}
vec3 TangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T) {
	return S * v.x + T * v.y + N * v.z;
}

const vec3 X = vec3(1.0, 0.0, 0.0);
const vec3 Y = vec3(0.0, 1.0, 0.0);
const vec3 Z = vec3(0.0, 0.0, 1.0);
mat3 ConstructTBN(const vec3 N) {
    vec3 T = (abs(N.y) < 0.9999) ? normalize(cross(Y, N)) : normalize(cross(X, N));
    vec3 B = -normalize(cross(N, T));
    return mat3(T, N, B);
}
vec3 WorldToLocal(vec3 vWorld, mat3 tbn) {
    return transpose(tbn) * vWorld;
}
vec3 LocalToWorld(vec3 vLocal, mat3 tbn) {
    return tbn * vLocal;
}
#endif