#ifndef SSSR_SAMPLING_GLSL
#define SSSR_SAMPLING_GLSL

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI    = 3.14159265358979323846;
const float TwoPI = 6.28318530717958647692;
const float InvPI = 0.31830988618379067154;
#endif

/* 
    [Sampling the GGX Distribution of Visible Normals] https://jcgt.org/published/0007/04/01/paper.pdf 
*/
vec3 SampleGGXVNDF(vec3 Ve, float ax, float ay, float U1, float U2) {
    vec3 Vh = normalize(vec3(ax * Ve.x, Ve.y, ay * Ve.z));
    float lensq = Vh.x * Vh.x + Vh.z * Vh.z;
    vec3 T1 = lensq > 0.0 ? vec3(-Vh.z, 0.0, Vh.x) * inversesqrt(lensq) : vec3(1.0, 0.0, 0.0);
    vec3 T2 = cross(Vh, T1);
    float r = sqrt(U1);
    float phi = 2.0 * PI * U2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.y);
    t2 = (1.0 - s) * sqrt(max(0.0, 1.0 - t1 * t1)) + s * t2;
    vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;
    vec3 Ne = normalize(vec3(ax * Nh.x, max(0.0, Nh.y), ay * Nh.z));
    return Ne;
}

// world axis(right-handed)
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
vec3 LocalToWorld(in out vec3 wi_local, vec3 normal) {
    vec3 t = (abs(normal.y) < 0.9999) ? normalize(cross(Y, normal)) : normalize(cross(X, normal));
    vec3 b = normalize(cross(normal, t));
    return vec3(
        wi_local.x * t.x + wi_local.y * normal.x + wi_local.z * b.x,
        wi_local.x * t.y + wi_local.y * normal.y + wi_local.z * b.y,
        wi_local.x * t.z + wi_local.y * normal.z + wi_local.z * b.z
    );
}

#endif