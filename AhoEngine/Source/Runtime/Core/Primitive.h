#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "AABB.h"

namespace Aho {
	struct PTMaterial {
		PTMaterial() = default;
		PTMaterial(const glm::vec3& albedo, const glm::vec3& pbr)
			: m_Albedo(albedo), m_PBR(pbr) { }
		glm::vec3 m_Albedo{ 0.95f, 0.95f, 0.95f };
		glm::vec3 m_PBR{ 0.85f, 0.1f, 0.0f }; // Roughness, Metalic
	};


	class Primitive {
	public:
		Primitive() = default;

		Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: m_v0(v0), m_v1(v1), m_v2(v2) {

			m_ov0.normal = m_v0.normal;
			m_ov0.position = m_v0.position;

			m_ov1.normal = m_v1.normal;
			m_ov1.position = m_v1.position;

			m_ov2.normal = m_v2.normal;
			m_ov2.position = m_v2.position;

			const auto& p0 = m_v0.position;
			const auto& p1 = m_v1.position;
			const auto& p2 = m_v2.position;

			glm::vec3 minPoint = glm::min(glm::min(p0, p1), p2);
			glm::vec3 maxPoint = glm::max(glm::max(p0, p1), p2);
			m_AABB = AABB(minPoint, maxPoint);
		}

		AABB GetAABB() const;
		void ApplyTransform(const glm::mat4& transform);
		glm::vec3 GetCentroid() const { return GetAABB().GetCentroid(); }

		Vertex GetVertex(int num) const { return num == 0 ? m_v0 : num == 1 ? m_v1 : m_v2; }

		bool Intersect(const Ray& ray) const;
	private:
		// stores the original position and normal
		struct oVertex {
			oVertex() = default;
			glm::vec3 position;
			glm::vec3 normal;
		} m_ov0, m_ov1, m_ov2;

		AABB m_AABB;
		Vertex m_v0;
		Vertex m_v1;
		Vertex m_v2;

		int meshID{ -1 };
	};
}

// trash
//	Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2, const PTMaterial& mat0, const PTMaterial& mat1, const PTMaterial& mat2)
//		: m_v0(v0), m_v1(v1), m_v2(v2), m_Material0(mat0), m_Material1(mat1), m_Material2(mat2) {

//	m_ov0.normal	= m_v0.normal;
//	m_ov0.position	= m_v0.position;
//	m_ov0.normal	= m_v0.normal;
//	m_ov0.position	= m_v0.position;
//	m_ov0.normal	= m_v0.normal;
//	m_ov0.position	= m_v0.position;
//}
//PTMaterial GetMaterial(int num) { return num == 0 ? m_Material0 : num == 1 ? m_Material1 : m_Material2; }
