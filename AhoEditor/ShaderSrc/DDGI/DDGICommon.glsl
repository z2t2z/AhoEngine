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

// Probe classification states
#define RTXGI_DDGI_PROBE_STATE_ACTIVE 0     // probe shoots rays and may be sampled by a front facing surface or another probe (recursive irradiance)
#define RTXGI_DDGI_PROBE_STATE_INACTIVE 1   // probe doesn't need to shoot rays, it isn't near a front facing surface#define RTXGI_DDGI_NUM_FIXED_RAYS 32

// The number of fixed rays that are used by probe relocation and classification.
// These rays directions are always the same to produce temporally stable results.
#define RTXGI_DDGI_NUM_FIXED_RAYS 32

// RTXGI
#define DDGI_IRRADIANCE_TEXSIZE 6
#define DDGI_DISTANCE_TEXSIZE 14

struct DDGIVolumeDescGPU {
    float probeSpacing;
    vec3 origin;
    vec3 probeScrollOffsets;
    int raysPerProbe;
    float hysteresis;
    ivec3 probeCounts;
    mat3 randomRotation;
    sampler2D DDGIIrradiance;
    sampler2D DDGIDistance;
};

struct DDGIPayload {
    vec3 radiance;
    float t;
};

bool g_DEBUG_R = false;
bool g_DEBUG_G = false;
bool g_DEBUG_B = false;

#endif