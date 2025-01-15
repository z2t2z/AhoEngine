#ifndef SSBO_GLSL
#define SSBO_GLSL


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
layout(std430, binding = 5) buffer Textures {
    TextureHandles s_TextureHandles[];
};


#endif