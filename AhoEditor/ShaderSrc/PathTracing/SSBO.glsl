#ifndef SSBO_GLSL
#define SSBO_GLSL

#include "DataStructs.glsl"

// SSBO 
layout(std430, binding = 0) buffer TLASNode {
    BVHNode s_tNodes[];
};
layout(std430, binding = 1) buffer TLASPrimitives {
    PrimitiveDesc s_tPrimitives[];
};
layout(std430, binding = 2) buffer BLASNode {
    BVHNode s_bNodes[];
};
layout(std430, binding = 3) buffer BLASPrimitives {
    PrimitiveDesc s_bPrimitives[];
};
layout(std430, binding = 4) buffer Offset {
    OffsetInfo s_OffsetInfo[];
};
layout(std140, binding = 5) buffer Textures {
    TextureHandles s_TextureHandles[];
};

// wavefront
layout(std430, binding = 6) buffer QueueCounter {
    uint s_QueueCounter;
};
layout(std430, binding = 7) buffer PayloadQueue0 {
    Payload s_Payload0[];
};
layout(std430, binding = 8) buffer PayloadQueue1 {
    Payload s_Payload1[];
};
layout(std430, binding = 9) buffer DispatchIndirectBuffer {
    DispatchBuffer s_DispatchParams[];
};

#endif