#ifndef GGX_SAMPLING_GLSL
#define GGX_SAMPLING_GLSL

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
// Takes in the sampled half vector H and returns the GGX distribution function D(H)
float D_GGX(vec3 H, float ax, float ay) {
    float a = H.x / ax;
    float b = H.z / ay;
    float denominator = Sqr(a) + Sqr(b) + Sqr(H.y);
    return 1.0 / (PI * ax * ay * Sqr(denominator));
}
float Lambda(vec3 V, float ax, float ay) {
    float Tmp = sqrt(1.0 + (Sqr(ax * V.x) + Sqr(ay * V.z)) / Sqr(V.y));
    return (-1.0 + Tmp) / 2.0;
}
// Takes in V and returns the shadowing-masking function G1(V)
float G1(vec3 V, float ax, float ay) {
    return 1.0 / (1.0 + Lambda(V, ax, ay));
}
// Dv(Ni) in the paper, return the pdf of sampling the half vector H
float VNDFHPdf(vec3 V, vec3 H, float ax, float ay) {
    float G1v = G1(V, ax, ay);
    float D = D_GGX(H, ax, ay);
    float pdf = G1v * max(0.0, dot(V, H)) * D / V.y;
    return pdf;
}
// PDF of sampling outgoing direction L = reflect(-V, H)
float GGXVNDFLPdf(in vec3 V, in vec3 H, in vec3 L, float ax, float ay) {
    if (L.y <= 0.0) {
        return 0.0; // not sure
    }
    float Dv = VNDFHPdf(V, H, ax, ay);
    float pdf = Dv / (4.0 * max(dot(V, H), 1e-6));
    return pdf;
}

#endif