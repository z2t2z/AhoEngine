#ifndef DDGI_COMMON_GLSL
#define DDGI_COMMON_GLSL

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI = 3.14159265358979323846;
const float TwoPI = 2 * PI;
const float InvPI   = 0.31830988618379067154;
const float Inv2PI  = 0.15915494309189533577;
const float Inv4PI  = 0.07957747154594766788;
#endif

struct DDGIVolumeDescGPU {
    float probeSpacing;
    uint raysPerProbe;
    uint probeCount;
    uint probeCountX;
    uint probeCountY;
    uint probeCountZ;
};


#endif