#ifndef PT_UNIFORMS_GLSL
#define PT_UNIFORMS_GLSL

// Uniforms
uniform int u_SrcWidth;
uniform int u_SrcHeight;

uniform int u_Frame;
// uniform sampelr2D u_PathTracingAccumulate;

// Constants
const float PI = 3.14159265358979323846;
const float InvPI = 1.0 / PI;
const float FLT_MAX = 1E20;

const int MAX_MESHES_CNT = 32;
const int TLAS_STACK_DEPTH = MAX_MESHES_CNT; 
const int BLAS_STACK_DEPTH = 64;
const int SHADING_STACK_DEPTH = 5;

const int LI_MAX_DEPTH = 5;

// Material Masks
const int EmptyMask		   = 0;
const int AlbedoMapMask	   = 1 << 0;
const int NormalMapMask	   = 1 << 1;
const int RoughnessMapMask = 1 << 2;
const int MetallicMapMask  = 1 << 3;
const int AOMapMask		   = 1 << 4;
const int AllMask          = AlbedoMapMask | NormalMapMask | RoughnessMapMask | MetallicMapMask | AOMapMask;

#endif
