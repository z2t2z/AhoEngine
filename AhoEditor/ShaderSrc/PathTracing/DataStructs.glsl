#ifndef DATA_STRUCTS_GLSL
#define DATA_STRUCTS_GLSL

#include "../Common/Material.glsl"

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
    int meshId; // not used
    int offset; // not used
};

struct Vertex {
    vec3 position;      float u;
    vec3 normal;        float v;
    vec3 tangent;       float _padding;
};

struct PrimitiveDesc {
    BBox bbox;
    Vertex v[3];
    int meshId; 
    int id;             // not used
    int primId;         // not used
    int materialMask;   // not used
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
    float cosTheta;
    float eta;
    vec3 N;
    vec3 pos;
    Material material;
};

State InitState() {
    State state;
    state.eta = 1.5;
    state.cosTheta = 1.0;
    state.N = vec3(0.0, 1.0, 0.0);
    state.pos = vec3(0.0);
    return state;
}

// TODO: No need
struct DotProducts {
    float LdotH;
    float LdotN;
    float VdotN;
    float VdotH;
    float LdotV;
};

void SetDotProducts(vec3 L, vec3 V, vec3 H, vec3 N, out DotProducts dp) {
    dp.LdotH = dot(L, H);
    dp.LdotN = L.y; 
    dp.VdotN = V.y; 
    dp.VdotH = dot(V, H);
    dp.LdotV = dot(L, V);
}

struct TextureHandles {
    layout(bindless_sampler) sampler2D albedoHandle;
    layout(bindless_sampler) sampler2D normalHandle;

    layout(bindless_sampler) sampler2D metallicHandle;
    layout(bindless_sampler) sampler2D roughnessHandle;

    vec3 baseColor;
    float subsurface;

    vec3 emissive;
    float emissiveScale;

    float metallic;
    float specular;
    float specTint;
    float roughness;

    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    
    float clearcoatGloss;
    float specTrans;
    float ior;
    float ax;

    float ay;
    float ao; // Not used
    float padding0;
    float padding1;
};

#endif