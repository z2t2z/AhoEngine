#ifndef PT_FUNCTIONS_GLSL
#define PT_FUNCTIONS_GLSL

#include "./DataStructs.glsl"
#include "./SSBO.glsl"
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

bool IntersectPrimitive(Ray ray, PrimitiveDesc p, inout TempHitInfo tinfo) {
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


void RayTracePrimitive(Ray ray, int id, inout HitInfo info) {
    int nodeOffset = s_OffsetInfo[id].nodeOffset;
    int primOffset = s_OffsetInfo[id].primtiveOffset;

    int stk[BLAS_STACK_DEPTH];
    int ptr = -1;
    stk[++ptr] = 0;
    while (ptr >= 0 && ptr < BLAS_STACK_DEPTH) {
        int u = stk[ptr--] + nodeOffset;
        BVHNode bNode = s_bNodes[u];
        float t;
        if (!IntersectBbox(ray, bNode.bbox, t)) {
            continue;
        }
        if (info.hit && t > info.t) {
            continue;
        }
        if (IsLeaf(bNode)) {
            // Check all primitives this node contains
            int i = bNode.firstPrimsIdx;
            int primsCnt = bNode.primsCnt;
            for (int cnt = 0; cnt < primsCnt; ++i, ++cnt) {
                PrimitiveDesc p = s_bPrimitives[i + primOffset];
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
            if (ptr + 2 >= BLAS_STACK_DEPTH) {
                // info.hit = false;
                // info.exceeded = true;
                break;
            }
            stk[++ptr] = bNode.right;
            stk[++ptr] = bNode.left;
        }
    }  
}

// Find the nearest primitive
void RayTrace(Ray ray, inout HitInfo info) {
    int stk[TLAS_STACK_DEPTH];
    int ptr = -1;
    stk[++ptr] = 0;
    while (ptr >= 0 && ptr < TLAS_STACK_DEPTH) {
        int u = stk[ptr--];
        BVHNode tNode = s_tNodes[u];
        float t;
        if (!IntersectBbox(ray, tNode.bbox, t)) {
            continue;
        }
        if (info.hit && t > info.t) {
            continue;
        }
        if (IsLeaf(tNode)) {
            PrimitiveDesc p = s_tPrimitives[tNode.firstPrimsIdx];
            RayTracePrimitive(ray, p.meshId, info);
        } else {
            if (ptr + 2 >= TLAS_STACK_DEPTH) {
                break;
            }
            stk[++ptr] = tNode.left;
            stk[++ptr] = tNode.right;
        }
    }
}

// Checks if a ray has any intersection
bool _AnyHit(Ray ray, int id, const float tNearest) {
    int nodeOffset = s_OffsetInfo[id].nodeOffset;
    int primOffset = s_OffsetInfo[id].primtiveOffset;
    int meshId = s_bNodes[nodeOffset].meshId;

    int stk[BLAS_STACK_DEPTH];
    int ptr = -1;
    stk[++ptr] = 0;

    while (ptr >= 0 && ptr < BLAS_STACK_DEPTH) {
        int u = stk[ptr--] + nodeOffset;

        BVHNode bNode = s_bNodes[u];
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
                PrimitiveDesc p = s_bPrimitives[i + primOffset];
                TempHitInfo tempInfo;
                if (IntersectPrimitive(ray, p, tempInfo)) {
                    if (tempInfo.t <= tNearest) {
                        return true;
                    }
                }
            }
        } else {
            if (ptr + 2 >= BLAS_STACK_DEPTH) {
                break;
            }
            stk[++ptr] = bNode.left;
            stk[++ptr] = bNode.right;
        }
    }
    return false;
}

// Checks if a ray has any intersection, given distance
bool AnyHit(Ray ray, const float tNearest) {
    int stk[TLAS_STACK_DEPTH];
    int ptr = -1;
    stk[++ptr] = 0;
    while (ptr >= 0 && ptr < TLAS_STACK_DEPTH) {
        int u = stk[ptr--];

        BVHNode tNode = s_tNodes[u];
        float t;
        if (!IntersectBbox(ray, tNode.bbox, t)) {
            continue;
        }
        if (t > tNearest) {
            continue;
        }

        if (IsLeaf(tNode)) {
            PrimitiveDesc p = s_tPrimitives[tNode.firstPrimsIdx];
            if (_AnyHit(ray, p.id, tNearest)) {
                return true;
            }
        } else {
            if (ptr + 2 >= TLAS_STACK_DEPTH) {
                break;
            }
            stk[++ptr] = tNode.right;
            stk[++ptr] = tNode.left;
        }
    }
    return false;
}

// Return true if there is no occlusion between from and to
bool VisibilityTest(vec3 from, vec3 to) {
#ifndef OPT_SHADOW_TEST
    return true;
#endif
    Ray ray;
    ray.direction = normalize(to - from);
    ray.origin = from + 0.01 * ray.direction;
    if (AnyHit(ray, length(to - from))) {
        return false;
    }
    return true;
}

#endif