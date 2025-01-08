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

const int TEST = 512;

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
                break;
            }
            stk[++ptr] = bNode.right;
            stk[++ptr] = bNode.left;
        }
    }  
}

// Find the nearest primitive
void RayTrace(Ray ray, inout HitInfo info) {
    int stk[TEST];
    int ptr = -1;
    stk[++ptr] = 0;
    while (ptr >= 0 && ptr < TEST) {
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
            if (ptr + 2 >= TEST) {
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
    // ivec2 screenSize = ivec2(imageSize(outputImage));
    // if (pixelCoord.x >= screenSize.x || pixelCoord.y >= screenSize.y) {
    //     return;
    // }
    Ray ray = GetRayFromScreenSpace(
            vec2(pixelCoord), 
            vec2(imageSize(outputImage))
        );
    // Ray ray = RayCasting(pixelCoord.x, pixelCoord.y, vec2(imageSize(outputImage)));

    HitInfo info;
    info.t = -FLT_MAX;
    info.meshId = -1;
    info.hit = false;
    {
        // PrimitiveDesc p;
        // Vertex v[3];
        // v[0].position = vec3(0.0, 0.0, 0.0);
        // v[1].position = vec3(1.0, 0.0, 0.0);
        // v[2].position = vec3(0.0, 1.0, 0.0);
        // p.v[0] = v[0];
        // p.v[1] = v[1];
        // p.v[2] = v[2];

        // float t;
        // if (IntersectPrimitive(ray, p, t)) {
        // 	info.hit = true;
        // }
    }

    RayTrace(ray, info);
    // PrimitiveDesc p = s_tPrimitives[3];
    
    // info.hit = true;
    // RayTracePrimitive(ray, 0, info);
	// for (int i = 0; i < 200; i++) {
        // BVHNode bNode = s_bNodes[i];
        // float t;
        // if (IsLeaf(bNode) && IntersectBbox(ray, bNode.bbox, t)) {
        //     int i = bNode.firstPrimsIdx;
        //     int primsCnt = bNode.primsCnt;
        //     for (int cnt = 0; cnt < primsCnt; ++i, ++cnt) {
        //         const PrimitiveDesc p = s_bPrimitives[i + 12];
        //         float t;
        //         if (IntersectPrimitive(ray, p, t)) {
        //             info.hit = true;
        //             break;
        //         }
        //     }
        // }
        // if (info.hit) {
        //     break;
        // }
		// PrimitiveDesc prim = s_bPrimitives[i];
		// float t;
		// if (IntersectPrimitive(ray, prim, t)) {
		// 	info.hit = true;
		// }
	// }


    vec3 outColor = info.hit ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 0.0);

    imageStore(outputImage, ivec2(gl_GlobalInvocationID.xy), vec4(outColor, 1.0));
}
