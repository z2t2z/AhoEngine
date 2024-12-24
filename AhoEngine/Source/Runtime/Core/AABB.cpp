#include "Ahopch.h"
#include "AABB.h"

namespace Aho {
	AABB AABB::Merge(const AABB& lhs, const AABB& rhs) {
		AABB res(lhs);
		res.Merge(rhs);
		return res;
	}

	AABB AABB::Merge(const AABB& lhs, const glm::vec3& p) {
		AABB res(lhs);
		res.m_Pmin = glm::min(res.m_Pmin, p);
		res.m_Pmax = glm::max(res.m_Pmax, p);
		return res;
	}

	bool AABB::Intersect(const Ray& ray) const {
        float tEnter = -FLT_MAX;
        float tExit = FLT_MAX;

        for (int i = 0; i < 3; ++i) {
            if (ray.direction[i] != 0.0f) {
                float tMin = (GetMin()[i] - ray.origin[i]) / ray.direction[i];
                float tMax = (GetMax()[i] - ray.origin[i]) / ray.direction[i];

                if (tMin > tMax) {
                    std::swap(tMin, tMax);
                }

                tEnter = std::max(tEnter, tMin);
                tExit = std::min(tExit, tMax);

                if (tEnter > tExit || tExit < 0.0f) {
                    return false;
                }
            }
            else if (ray.origin[i] < GetMin()[i] || ray.origin[i] > GetMax()[i]) {
                return false; // Ray is parallel and outside the slab
            }
        }

        return true;
	}
}
