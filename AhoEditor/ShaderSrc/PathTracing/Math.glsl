#ifndef MATH_GLSL
#define MATH_GLSL

// world axis(right-handed)
const vec3 X = vec3(1.0, 0.0, 0.0);
const vec3 Y = vec3(0.0, 1.0, 0.0);
const vec3 Z = vec3(0.0, 0.0, 1.0);
const vec3 _luminance_vec3 = vec3(0.212671, 0.715160, 0.072169);
float Luminance(vec3 c) {
    // return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
    return dot(c, _luminance_vec3);
}

float Sqr(float v) {
    return v * v;
}
bool IsInf(float v) {
    return isinf(v);
}
void Swap(inout float a, inout float b) {
    float c = a;
    a = b;
    b = c;
}

void SwapYZ(inout vec3 w) {
    float t = w.y;
    w.y = w.z;
    w.z = t;
}

// Spherical Geometry
float CosTheta(vec3 w) {
    return w.y;
}
float Cos2Theta(vec3 w) {
    return CosTheta(w) * CosTheta(w);
}
float AbsCosTheta(vec3 w) {
    return abs(w.y);
}
float Sin2Theta(vec3 w) { 
    return max(0.0, 1.0 - Cos2Theta(w)); 
}
float SinTheta(vec3 w) { 
    return sqrt(Sin2Theta(w)); 
}
float TanTheta(vec3 w) { 
    return SinTheta(w) / CosTheta(w); 
}
float Tan2Theta(vec3 w) { 
    return Sin2Theta(w) / Cos2Theta(w); 
}
float CosPhi(vec3 w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0.0) ? 1.0 : clamp(w.x / sinTheta, -1.0, 1.0);
}
float Cos2Phi(const vec3 w) { 
    return CosPhi(w) * CosPhi(w); 
}
float SinPhi(vec3 w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0.0) ? 0.0 : clamp(w.z / sinTheta, -1.0, 1.0);
}
float Sin2Phi(const vec3 w) { 
    return SinPhi(w) * SinPhi(w); 
}
bool SameHemisphere(const vec3 w, const vec3 wp) {
    return w.y * wp.y > 0;
}
vec3 SphericalDirection(float sinTheta, float cosTheta, float phi) {
    return vec3(sinTheta * cos(phi), cosTheta, sinTheta * sin(phi));
}
vec3 Faceforward(const vec3 n, const vec3 n2) {
    return (dot(n, n2) < 0.0) ? -n : n;
}

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
vec3 LocalToWorld(in out vec3 wi_local, vec3 normal) {
    vec3 t = (abs(normal.y) < 0.9999) ? normalize(cross(Y, normal)) : normalize(cross(X, normal));
    vec3 b = normalize(cross(normal, t));
    return vec3(
        wi_local.x * t.x + wi_local.y * normal.x + wi_local.z * b.x,
        wi_local.x * t.y + wi_local.y * normal.y + wi_local.z * b.y,
        wi_local.x * t.z + wi_local.y * normal.z + wi_local.z * b.z
    );
}

float LengthSquared(vec3 v) {
    return Sqr(v.x) + Sqr(v.y) + Sqr(v.z);
}
float LengthSquared(vec2 v) {
    return Sqr(v.x) + Sqr(v.y);
}


#endif