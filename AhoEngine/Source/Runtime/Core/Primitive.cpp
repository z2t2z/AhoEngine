#include "Ahopch.h"
#include "Primitive.h"

namespace Aho {
	AABB Primitive::GetAABB() const {
		return m_AABB;
	}

	void Primitive::ApplyTransform(const glm::mat4& transform) {
		glm::mat3 matrix3 = glm::transpose(glm::inverse(glm::mat3(transform)));
		m_v0.position = glm::vec3(transform * glm::vec4(m_ov0.position, 1.0f));
		m_v1.position = glm::vec3(transform * glm::vec4(m_ov1.position, 1.0f));
		m_v2.position = glm::vec3(transform * glm::vec4(m_ov2.position, 1.0f));

		m_v0.normal = matrix3 * m_ov0.normal;
		m_v1.normal = matrix3 * m_ov1.normal;
		m_v2.normal = matrix3 * m_ov2.normal;

		const auto& p0 = m_v0.position;
		const auto& p1 = m_v1.position;
		const auto& p2 = m_v2.position;

		glm::vec3 minPoint = glm::min(glm::min(p0, p1), p2);
		glm::vec3 maxPoint = glm::max(glm::max(p0, p1), p2);
		m_AABB = AABB(minPoint, maxPoint);
	}

	bool Primitive::Intersect(const Ray& ray) const {
        const glm::vec3& v0 = GetVertex(0).position;
        const glm::vec3& v1 = GetVertex(1).position;
        const glm::vec3& v2 = GetVertex(2).position;

        glm::vec3 E1, E2, S, S1, S2, result;
        E1 = v1 - v0;
        E2 = v2 - v0;
        S = ray.origin - v0;
        S1 = glm::cross(ray.direction, E2);
        S2 = glm::cross(S, E1);
        result = glm::vec3(glm::dot(S2, E2), glm::dot(S1, S), glm::dot(S2, ray.direction)) / glm::dot(S1, E1);
        float tnear = result.x;
        float u = result.y;
        float v = result.z;
        if (tnear > 0 && v >= 0 && v <= 1 && u >= 0 && u <= 1) {
            return true;
        }
        return false;

        //glm::vec3 edge1 = v1 - v0;
        //glm::vec3 edge2 = v2 - v0;

        //glm::vec3 h = glm::cross(ray.direction, edge2);
        //float a = glm::dot(h, edge1);

        //if (a > -1e-8 && a < 1e-8) {
        //    return false;
        //}

        //float f = 1.0f / a;
        //glm::vec3 s = ray.origin - v0;
        //float u = f * glm::dot(s, h);

        //if (u < 0.0f || u > 1.0f) {
        //    return false;
        //}

        //glm::vec3 q = glm::cross(s, edge1); //s.cross(edge1);
        //float v = f * glm::dot(ray.direction, q); // ray.direction.dot(q);

        //if (v < 0.0f || u + v > 1.0f) {
        //    return false;
        //}

        //float t = f * glm::dot(edge2, q); //edge2.dot(q);

        //if (t < 0.0f) {
        //    return false;
        //}

        //return true;
	}
}
