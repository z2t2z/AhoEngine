#include "Ahopch.h"
#include "Primitive.h"

namespace Aho {
	AABB Primitive::GetAABB() {
		if (m_Dirty) {
			m_Dirty = false;
			const auto& p0 = m_v0.position;
			const auto& p1 = m_v1.position;
			const auto& p2 = m_v2.position;

			glm::vec3 minPoint = glm::min(glm::min(p0, p1), p2);
			glm::vec3 maxPoint = glm::max(glm::max(p0, p1), p2);
			m_AABB = AABB(minPoint, maxPoint);
		}
		return m_AABB;
	}

	void Primitive::Update() {
		AHO_CORE_ASSERT(m_Transform);
		glm::mat4 matrix4 = m_Transform->GetTransform();
		glm::mat3 matrix3 = glm::transpose(glm::inverse(glm::mat3(matrix4)));
		m_v0.position = glm::vec3(matrix4 * glm::vec4(m_v0.position, 1.0f));
		m_v1.position = glm::vec3(matrix4 * glm::vec4(m_v1.position, 1.0f));
		m_v2.position = glm::vec3(matrix4 * glm::vec4(m_v2.position, 1.0f));

		m_v0.normal = matrix3 * m_v0.normal;
		m_v1.normal = matrix3 * m_v1.normal;
		m_v2.normal = matrix3 * m_v2.normal;
	}
}
