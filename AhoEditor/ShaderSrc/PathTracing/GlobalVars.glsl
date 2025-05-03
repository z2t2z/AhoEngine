#ifndef GLOBAL_VARS_GLSL
#define GLOBAL_VARS_GLSL

// Uniforms
uniform int u_SrcWidth;
uniform int u_SrcHeight;

uniform ivec2 u_Resolution;
uniform ivec2 u_TileSize;
uniform ivec2 u_TileNum;

uniform int u_Frame;

// Infinite area light, ibl 
uniform sampler2D u_EnvLight;

// Constants
const float EPS = 0.000001;

const float PI      = 3.14159265358979323846;
const float InvPI   = 0.31830988618379067154;
const float Inv2PI  = 0.15915494309189533577;
const float Inv4PI  = 0.07957747154594766788;
const float FLT_MAX = 1E20;

const int TLAS_STACK_DEPTH = 8; 
const int BLAS_STACK_DEPTH = 24;
const int SHADING_STACK_DEPTH = 5;


#endif
