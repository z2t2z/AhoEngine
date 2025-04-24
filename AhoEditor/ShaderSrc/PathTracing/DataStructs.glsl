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
    int meshId;
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
    int meshId; 
    int id;
    int primId;
    int materialMask; // not used
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

struct Material {
    vec3 baseColor;
    float subsurface;

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
};

struct State {
    float cosTheta;
    float pdf;
    float eta;
    vec3 N;
    vec3 pos;
    Material material;
};

State InitState() {
    State state;
    state.pdf = 1.0;
    state.eta = 1.5;
    state.cosTheta = 1.0;
    state.N = vec3(0.0, 1.0, 0.0);
    state.pos = vec3(0.0);
    return state;
}

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

// Weired bugs
struct TextureHandles {
    layout(bindless_sampler) sampler2D albedoHandle;
    layout(bindless_sampler) sampler2D normalHandle;

    layout(bindless_sampler) sampler2D metallicHandle;
    layout(bindless_sampler) sampler2D roughnessHandle;

    vec3 baseColor;
    float subsurface;

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
    float padding0;
    float padding1;
    float padding2;
};


#endif