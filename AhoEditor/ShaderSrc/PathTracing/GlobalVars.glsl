#ifndef PT_UNIFORMS_GLSL
#define PT_UNIFORMS_GLSL

// Uniforms
uniform int u_SrcWidth;
uniform int u_SrcHeight;

// Constants
const float PI = 3.14159265358979323846;
const float FLT_MAX = 1E20;

const int MAX_MESHES_CNT = 128;
const int TLAS_STACK_DEPTH = MAX_MESHES_CNT; 
const int BLAS_STACK_DEPTH = 512;
const int SHADING_STACK_DEPTH = 5;

#endif
