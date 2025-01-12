#type compute
#version 460

#include "./PathTracingCommon.glsl"
#include "./GlobalVars.glsl"
#include "../UniformBufferObjects.glsl"
#include "./IntersectionTest.glsl"

layout(binding = 0, rgba32f) uniform image2D outputImage;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

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
                float t;
                if (IntersectPrimitive(ray, p, t)) {
                    if (info.t < 0 || info.t > t) {
                        info.t = t;
                        info.hit = true;
                    }
                }
            }
        } else {
            if (ptr + 2 >= BLAS_STACK_DEPTH) {
                info.hit = false;
                info.exceeded = true;
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
            
            RayTracePrimitive(ray, p.id, info);
 
        } else {
            if (ptr + 2 >= TLAS_STACK_DEPTH) {
                info.exceeded = true;
                info.hit = false;
                break;
            }
            stk[++ptr] = tNode.right;
            stk[++ptr] = tNode.left;
        }
    }

}


// vec3 Shading() {
//     Ray ray = GetRayFromScreenSpace();
//     HitInfo info;
//     RayTrace(ray, info);
//     vec3 color = vec3(0.0, 0.0, 0.0);
//     if (info.t > 0) {
//         int id = info.meshId;
//         color += EvaluateDirectLighting(id);
//         Ray reflectRay = GetReflectedRay(ray, id);
//         color += EvaluateIndirectlight(reflectRay);
//     } else {
//         color = GetAtmosphereColor();
//     }
//     return color;
// }

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    Ray ray = GetRayFromScreenSpace(
            vec2(pixelCoord), 
            vec2(imageSize(outputImage))
        );
    // Ray ray = RayCasting(pixelCoord.x, pixelCoord.y, vec2(imageSize(outputImage)));

    HitInfo info;
    info.t = -FLT_MAX;
    info.meshId = -1;
    info.hit = false;
    info.exceeded = false;

    RayTrace(ray, info);

    vec3 outColor = info.hit ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 0.0);

    if (info.exceeded) {
        outColor = vec3(0.0, 0.0, 1.0);
    }

    imageStore(outputImage, ivec2(gl_GlobalInvocationID.xy), vec4(outColor, 1.0));
}
