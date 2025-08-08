#ifndef GLOBAL_VARS_GLSL
#define GLOBAL_VARS_GLSL

// Constants
const float EPS = 0.000001;

#ifndef CONSTANT_PI
#define CONSTANT_PI
const float PI      = 3.14159265358979323846;
const float TwoPI   = 2 * PI;
const float FourPI  = 4 * PI;
const float InvPI   = 0.31830988618379067154;
const float Inv2PI  = 0.15915494309189533577;
const float Inv4PI  = 0.07957747154594766788;
#endif

const float FLT_MAX = 1E20;

// Bvh traversal settings
const int TLAS_STACK_DEPTH = 32; 
const int BLAS_STACK_DEPTH = 32;
// Global int[], much faster
#define MAX_RAYTRACE_TLAS_NUMS 128
#define MAX_RAYTRACE_BLAS_NUMS 128
int BLAS_STK[BLAS_STACK_DEPTH];
int TLAS_STK[TLAS_STACK_DEPTH];

#endif
