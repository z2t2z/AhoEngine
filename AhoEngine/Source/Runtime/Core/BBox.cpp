#include "Ahopch.h"
#include "BBox.h"

namespace Aho {
	BBox BBox::Merge(const BBox& lhs, const BBox& rhs) {
		BBox res(lhs);
		res.Merge(rhs);
		return res;
	}

	BBox BBox::Merge(const BBox& lhs, const glm::vec3& p) {
		BBox res(lhs);
		res.m_Pmin = glm::min(res.m_Pmin, p);
		res.m_Pmax = glm::max(res.m_Pmax, p);
		return res;
	}



	bool BBox::Intersect(const Ray& ray) const {
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
    bool BBox::IntersectNearest(const Ray& ray, float& t) const {
        t = FLT_MAX;
        float tEnter = -t;
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
