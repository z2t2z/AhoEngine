#include "Ahopch.h"
#include "Math.h"
#include "Runtime/Core/BVH.h"

namespace Aho {
	bool Intersect(const Ray& ray, const AABB& aabb) {
        float tEnter = -FLT_MAX;
        float tExit = FLT_MAX;

        for (int i = 0; i < 3; ++i) {
            if (ray.direction[i] != 0.0f) {
                float tMin = (aabb.GetMin()[i] - ray.origin[i]) / ray.direction[i];
                float tMax = (aabb.GetMax()[i] - ray.origin[i]) / ray.direction[i];

                if (tMin > tMax) {
                    std::swap(tMin, tMax);
                }

                tEnter = std::max(tEnter, tMin);
                tExit = std::min(tExit, tMax);

                if (tEnter > tExit || tExit < 0.0f) {
                    return false;
                }
            }
            else if (ray.origin[i] < aabb.GetMin()[i] || ray.origin[i] > aabb.GetMax()[i]) {
                return false; // Ray is parallel and outside the slab
            }
        }

        return true; 
	}

	// Möller–Trumbore algo to test if a ray intersects a triangle
	std::optional<IntersectResult> Intersect(const Ray& ray, const Primitive* primitive) {
        // 三角形的顶点
        const glm::vec3& v0 = primitive->GetVertex(0).position;
        const glm::vec3& v1 = primitive->GetVertex(1).position;
        const glm::vec3& v2 = primitive->GetVertex(2).position;

        // 计算边
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        // 计算 determinant
        glm::vec3 h = glm::cross(ray.direction, edge2);
        float a = glm::dot(h, edge1);

        // 如果 a 过于接近 0，说明光线平行于三角形
        if (a > -1e-8 && a < 1e-8) {
            return std::nullopt; // 没有相交
        }

        float f = 1.0f / a;
        glm::vec3 s = ray.origin - v0;
        float u = f * glm::dot(s, h);

        // 检查 u 参数是否在 [0, 1] 范围内
        if (u < 0.0f || u > 1.0f) {
            return std::nullopt; // 没有相交
        }

        glm::vec3 q = glm::cross(s, edge1); //s.cross(edge1);
        float v = f * glm::dot(ray.direction, q); // ray.direction.dot(q);

        // 检查 v 参数是否在 [0, 1] 范围内，并且 u+v <= 1
        if (v < 0.0f || u + v > 1.0f) {
            return std::nullopt; // 没有相交
        }

        // 计算光线参数 t
        float t = f * glm::dot(edge2, q); //edge2.dot(q);

        // 如果 t 为负数，说明相交点在光线起点之前
        if (t < 0.0f) {
            return std::nullopt; // 没有相交
        }

        // 计算相交点的具体信息
        glm::vec3 intersectionPoint = ray.origin + ray.direction * t;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));// edge1.cross(edge2).normalized();  // 法线
        glm::vec3 barycentric(u, v, 1 - u - v);              // 重心坐标

        // 根据三角形的顶点 UV 进行插值
        glm::vec2 interpolatedUV = primitive->GetVertex(0).uv * barycentric.z
                                    + primitive->GetVertex(1).uv * barycentric.x
                                    + primitive->GetVertex(2).uv * barycentric.y;

        // 返回相交信息
        return IntersectResult{ t, intersectionPoint, normal, barycentric, interpolatedUV };
	}

    Ray GetRayFromScreenSpace(const glm::vec2& coords, const glm::vec2& resolution, const glm::vec3& camPos, const glm::mat4& projInv, const glm::mat4& viewInv) {
        glm::vec4 worldPos = glm::vec4(coords / resolution * glm::vec2(2.0f) - glm::vec2(1.0f), -1.0, 1.0);
        worldPos = projInv * worldPos;
        //AHO_CORE_ASSERT(worldPos.w == 1.0f);
        if (worldPos.w != 0.0f) {
            worldPos /= worldPos.w;
        }
        worldPos = viewInv * worldPos;

        return Ray(camPos, glm::normalize(glm::vec3(worldPos) - camPos));
    }

}