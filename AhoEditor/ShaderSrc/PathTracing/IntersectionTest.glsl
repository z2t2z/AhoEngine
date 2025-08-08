#ifndef PT_FUNCTIONS_GLSL
#define PT_FUNCTIONS_GLSL

#include "GlobalVars.glsl"
#include "DataStructs.glsl"
#include "SSBO.glsl"
#include "../Common/UniformBufferObjects.glsl"

bool IsLeaf(BVHNode node) {
    return node.left == -1 && node.right == -1 && node.firstPrimsIdx >= 0 && node.primsCnt > 0;
}

bool IntersectBbox(Ray ray, BBox bbox, inout float tEnter) {
    tEnter = -FLT_MAX;
    float tExit = FLT_MAX;
    for (int i = 0; i < 3; ++i) {
        if (ray.direction[i] != 0.0f) {
            float tMin = (bbox.pMin[i] - ray.origin[i]) / ray.direction[i];
            float tMax = (bbox.pMax[i] - ray.origin[i]) / ray.direction[i];
            if (tMin > tMax) {
                float t = tMax;
                tMax = tMin;
                tMin = t;
            }
            tEnter = max(tEnter, tMin);
            tExit = min(tExit, tMax);
            if (tEnter > tExit || tExit < 0.0f) {
                // tEnter = -FLT_MAX;
                return false;
            }
        }
        else if (ray.origin[i] < bbox.pMin[i] || ray.origin[i] > bbox.pMax[i]) {
            // tEnter = -FLT_MAX;
            return false; // Ray is parallel and outside the slab
        }
    }
    return true; 
}

bool IntersectPrimitive(const Ray ray, const PrimitiveDesc p, inout TempHitInfo tinfo) {
    const vec3 v0 = p.v[0].position;
    const vec3 v1 = p.v[1].position;
    const vec3 v2 = p.v[2].position;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(ray.direction, edge2);
    float a = dot(h, edge1);
    if (a > -1e-8 && a < 1e-8) {
        return false;
    }
    float f = 1.0f / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    } 
    vec3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    float t = f * dot(edge2, q);
    if (t < 0.0f) {
        return false;
    }
    tinfo.t = t;
    tinfo.uv = vec2(u, v);
    return true;
}

void RayToLocal(inout Ray ray, mat3 inv3x3, mat4 transform) {
}

void RayTracePrimitive(Ray ray, int id, inout HitInfo info) {
    int nodeOffset = s_OffsetInfo[id].nodeOffset;
    int primOffset = s_OffsetInfo[id].primtiveOffset;
    int ptr = -1;
    BLAS_STK[++ptr] = 0;
    int loop = MAX_RAYTRACE_BLAS_NUMS;
    BVHNode bNode;
    PrimitiveDesc p;
    while (loop >= 0 && ptr >= 0 && ptr < BLAS_STACK_DEPTH) {
        int u = BLAS_STK[ptr--] + nodeOffset;
        bNode = s_bNodes[u];
        float t;
        if (!IntersectBbox(ray, bNode.bbox, t)) continue;
        if (info.hit && t > info.t) continue;
        if (IsLeaf(bNode)) {
            // Check all primitives this node contains
            int i = bNode.firstPrimsIdx;
            int primsCnt = bNode.primsCnt;
            for (int cnt = 0; cnt < primsCnt; ++i, ++cnt) {
                p = s_bPrimitives[i + primOffset];
                TempHitInfo tempInfo;
                if (IntersectPrimitive(ray, p, tempInfo)) {
                    if (info.t < 0 || info.t > tempInfo.t) {
                        info.t = tempInfo.t;
                        info.uv = tempInfo.uv;
                        info.hit = true;
                        info.meshId = id;
                        info.globalPrimtiveId = i + primOffset;
                    }
                }
            }
        } else {
            if (ptr + 2 >= BLAS_STACK_DEPTH) break;
            BLAS_STK[++ptr] = bNode.right;
            BLAS_STK[++ptr] = bNode.left;
        }
        loop -= 1;
    }  
}

// Find the nearest primitive
void RayTrace(Ray ray, inout HitInfo info) {
    int ptr = -1;
    TLAS_STK[++ptr] = 0;
    int loop = MAX_RAYTRACE_TLAS_NUMS;
    BVHNode tNode;
    while (loop >= 0 && ptr >= 0 && ptr < TLAS_STACK_DEPTH) {
        int u = TLAS_STK[ptr--];
        tNode = s_tNodes[u];
        float t;
        if (!IntersectBbox(ray, tNode.bbox, t)) continue;
        if (info.hit && t > info.t) continue;
        if (IsLeaf(tNode)) {
            PrimitiveDesc p = s_tPrimitives[tNode.firstPrimsIdx];
            RayTracePrimitive(ray, p.meshId, info);
        } else {
            if (ptr + 2 >= TLAS_STACK_DEPTH) break;
            TLAS_STK[++ptr] = tNode.right;
            TLAS_STK[++ptr] = tNode.left;
        }
        loop -= 1;
    }
}

// Checks if a ray has any intersection
bool _AnyHit(Ray ray, int id, const float tNearest) {
    int nodeOffset = s_OffsetInfo[id].nodeOffset;
    int primOffset = s_OffsetInfo[id].primtiveOffset;
    int ptr = -1;
    BLAS_STK[++ptr] = 0;
    int loop = MAX_RAYTRACE_BLAS_NUMS;
    BVHNode bNode;
    PrimitiveDesc p;
    while (loop >= 0 && ptr >= 0 && ptr < BLAS_STACK_DEPTH) {
        int u = BLAS_STK[ptr--] + nodeOffset;
        bNode = s_bNodes[u];
        float t;
        if (!IntersectBbox(ray, bNode.bbox, t)) {
            continue;
        }
        if (t > tNearest) {
            continue;
        }
        if (IsLeaf(bNode)) {
            // Check all primitives this node contains
            int i = bNode.firstPrimsIdx;
            int primsCnt = bNode.primsCnt;
            for (int cnt = 0; cnt < primsCnt; ++i, ++cnt) {
                p = s_bPrimitives[i + primOffset];
                TempHitInfo tempInfo;
                if (IntersectPrimitive(ray, p, tempInfo)) {
                    if (tempInfo.t <= tNearest) {
                        return true;
                    }
                }
            }
        } else {
            if (ptr + 2 >= BLAS_STACK_DEPTH) break;
            BLAS_STK[++ptr] = bNode.right;
            BLAS_STK[++ptr] = bNode.left;
        }
        loop -= 1;
    }
    return false;
}

// Checks if a ray has any intersection, given distance
bool AnyHit(Ray ray, const float tNearest) {
    int ptr = -1;
    TLAS_STK[++ptr] = 0;
    int loop = MAX_RAYTRACE_TLAS_NUMS;
    while (loop >= 0 && ptr >= 0 && ptr < TLAS_STACK_DEPTH) {
        int u = TLAS_STK[ptr--];
        BVHNode tNode = s_tNodes[u];
        float t;
        if (!IntersectBbox(ray, tNode.bbox, t)) continue;
        if (t > tNearest) continue;

        if (IsLeaf(tNode)) {
            PrimitiveDesc p = s_tPrimitives[tNode.firstPrimsIdx];
            if (_AnyHit(ray, p.meshId, tNearest)) {
                return true;
            }
        } else {
            if (ptr + 2 >= TLAS_STACK_DEPTH) break;
            TLAS_STK[++ptr] = tNode.left;
            TLAS_STK[++ptr] = tNode.right;
        }
        loop -= 1;
    }
    return false;
}

bool IsOccluded(const Ray ray, const float t) {
    return AnyHit(ray, t);
}

#endif