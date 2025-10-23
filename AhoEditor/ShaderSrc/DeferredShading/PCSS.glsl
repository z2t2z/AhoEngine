#ifndef PCSS_GLSL
#define PCSS_GLSL

// -----------------------------
// PCSS helper functions (GLSL)
// -----------------------------
// Assumptions:
// - shadowMap: sampler2D depth texture containing depth from light (linear depth in same units as lightSpacePos.z)
// - lightSpacePos: vec3(uv.x, uv.y, depthFromLight) where uv in [0,1]
// - lightSize: approximate "radius" of area light in shadow-map UV units (e.g. 0.001 - 0.05). See notes below.
// - shadowMapResolution: float, e.g. 2048.0
// - bias: depth bias to avoid self-shadowing (normal bias or constant bias)
// - poissonDisk: array of sample offsets in [-1,1] used for sampling in UV space
// -----------------------------