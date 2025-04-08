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
    float metallic;
    float specTrans;
    float roughness;
    float subsurface;
    float ior;
    float clearcoat;
    float clearcoatGloss;
    float sheen;
// private:
    float ax;
    float ay;
}; 

struct State {
    vec3 baseColor; // albedo
    float cosTheta;
    float metallic;
    float roughness;
    float subsurface;
    float specular;
    float specTrans;
    float ax;
    float ay;
    float ior;
    float clearcoat;
    float clearcoatGloss;
    float pdf;
    float eta;
    vec3 uvw;
    vec3 N;
};

struct DotProducts {
    float LdotH;
    float LdotN;
    float VdotN;
    float VdotH;
    float LdotV;
};
void SetDotProducts(vec3 L, vec3 V, vec3 H, vec3 N, out DotProducts dp) {
    dp.LdotH = dot(L, H);
    dp.LdotN = dot(L, N);
    dp.VdotN = dot(V, N);
    dp.VdotH = dot(V, H);
    dp.LdotV = dot(L, V);
}

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