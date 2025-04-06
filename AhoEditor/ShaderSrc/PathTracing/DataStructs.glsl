#ifndef DATA_STRUCTS_GLSL
#define DATA_STRUCTS_GLSL

struct BBox {
    vec3 pMin;          float _padding0;
    vec3 pMax;          float _padding1;
};

struct OffsetInfo {
    int _padding;
    int offset;
    int nodeOffset;
    int primtiveOffset;
};

struct BVHNode {
    BBox bbox;               
    int left;
    int right;         
    int nodeIdx;             
    int firstPrimsIdx;       
    int primsCnt;            
    int axis;   // not used
    int meshId; // only valid for root node
    int offset;
};

struct Vertex {
    vec3 position;      float u;
    vec3 normal;        float v;
    vec3 tangent;       float _padding;
};

struct PrimitiveDesc {
    BBox bbox;
    Vertex v[3];
    int meshId; // not used
    int id;
    int primId;
    int materialMask;
};

struct Ray {
    vec3 origin;		float _padding0;
    vec3 direction;	    float _padding1;
};

struct HitInfo {
    int meshId;
    int globalPrimtiveId;
    vec2 uv; 
    float t;
    bool hit;
    bool exceeded;
};

HitInfo InitHitInfo() {
    HitInfo info;
    info.hit = false;
    info.t = -1.0;
    info.globalPrimtiveId = -1;
    info.meshId = -1;
    return info;
}

struct TempHitInfo {
    vec2 uv;
    float t;
};

struct State {
    vec3 baseColor; // albedo
    float metallic;
    float roughness;
    float subsurface;
    float specTrans;
    float specular;
    float specularTint;
    float sheenTint;
    float ax;
    float ay;
    float ior;
};

struct TextureHandles {
    layout(bindless_sampler) sampler2D albedo;
    layout(bindless_sampler) sampler2D normal;

    layout(bindless_sampler) sampler2D metallic;
    layout(bindless_sampler) sampler2D roughness;
};

bool IsLeaf(BVHNode node) {
    return node.left == -1 && node.right == -1 && node.firstPrimsIdx >= 0 && node.primsCnt > 0;
}


#endif