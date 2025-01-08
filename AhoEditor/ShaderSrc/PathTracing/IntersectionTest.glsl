#ifndef PT_FUNCTIONS_GLSL
#define PT_FUNCTIONS_GLSL

#include "./DataStructs.glsl"
#include "../UniformBufferObjects.glsl"

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

bool IntersectPrimitive(Ray ray, PrimitiveDesc p, inout float t) {
    t = -1.0;
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

    vec3 q = cross(s, edge1); //s.cross(edge1);
    float v = f * dot(ray.direction, q); // ray.direction.dot(q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    t = f * dot(edge2, q); //edge2.dot(q);

    if (t < 0.0f) {
        return false;
    }

    return true;
    // const vec3 v0 = p.v[0].position;
    // const vec3 v1 = p.v[1].position;
    // const vec3 v2 = p.v[2].position;

    // vec3 E1 = v1 - v0;
    // vec3 E2 = v2 - v0;
    // vec3 S = ray.origin - v0;
    // vec3 S1 = cross(ray.direction, E2);
    // vec3 S2 = cross(S, E1);
    // vec3 result = vec3(dot(S2, E2), dot(S1, S), dot(S2, ray.direction)) / dot(S1, E1);

    // float tnear = result.x;
    // t = tnear;
    // float u = result.y;
    // float v = result.z;
    
    // if (tnear > 0.0 && v >= 0.0 && v <= 1 && u >= 0.0 && u <= 1.0) {
    //     return true;
    // }
    // t = -1.0;
    // return false;
}


#endif