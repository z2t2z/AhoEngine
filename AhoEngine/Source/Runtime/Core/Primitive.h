#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "BBox.h"

namespace Aho {

	class BVHi;
	class PrimitiveCompliment;

	// Aligned with gpu
	struct alignas(16) PrimitiveDesc {
	public:
		PrimitiveDesc() = default;
		// BLAS Constructor
		PrimitiveDesc(const Vertex& v0, const Vertex& v1, const Vertex& v2, int id, int meshId = -1)
			: m_PrimId(id), m_MeshId(meshId) {
			m_Vertices[0] = v0;
			m_Vertices[1] = v1;
			m_Vertices[2] = v2;
			const auto& p0 = m_Vertices[0].position;
			const auto& p1 = m_Vertices[1].position;
			const auto& p2 = m_Vertices[2].position;
			glm::vec3 minPoint = Min(Min(p0, p1), p2);
			glm::vec3 maxPoint = Max(Max(p0, p1), p2);
			m_BBox = BBox(minPoint, maxPoint);
		}
		// ALTS Constructor
		PrimitiveDesc(const BVHi* blas, int offset);
		void ApplyTransform(const glm::mat4& transform, const PrimitiveCompliment& pc);

		BBox GetBBox() const;
		void SetBBox(const BBox& bbox) { m_BBox = bbox; }

		glm::vec3 GetCentroid() const { return GetBBox().GetCentroid(); }
		bool Intersect(const Ray& ray) const;
		bool IntersectNearest(const Ray& ray, float& t) const;
		Vertex GetVertex(int num) const { return m_Vertices[num]; }
		Vertex& GetVertex(int num) { return m_Vertices[num]; }
		int GetId() const { return m_Id; } // this id is for transform only
		void SetID(int id) { m_Id = id; }
		int GetPrimId() const { return m_PrimId; } // this id is for transform only

	private:
		BBox m_BBox;
		Vertex m_Vertices[3];
		int m_MeshId;
		int m_Id{ -1 };
		int m_PrimId{ -1 };
		float _padding;
		//glm::vec2 _padding0{ -1 };
	};


	// Store the original vertices and normals for transform
	struct PrimitiveCompliment {
		PrimitiveCompliment(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
			position[0] = v0.position;
			position[1] = v1.position;
			position[2] = v2.position;

			normal[0] = v0.normal;
			normal[1] = v1.normal;
			normal[2] = v2.normal;
		}
		glm::vec3 position[3];
		glm::vec3 normal[3];
	};

	//class Primitive {
	//public:
	//	Primitive() = default;
	//	Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2, int meshId = -1)
	//		: m_PrimDesc(v0, v1, v2, meshId) {

	//		m_ov0.normal = v0.normal;
	//		m_ov0.position = v0.position;

	//		m_ov1.normal = v1.normal;
	//		m_ov1.position = v1.position;

	//		m_ov2.normal = v2.normal;
	//		m_ov2.position = v2.position;
	//	}

	//	BBox GetBBox() const;
	//	void ApplyTransform(const glm::mat4& transform);
	//	glm::vec3 GetCentroid() const { return GetBBox().GetCentroid(); }

	//	Vertex GetVertex(int num) const { return m_PrimDesc.v[num]; }

	//	bool Intersect(const Ray& ray) const;

	//private:
	//	PrimitiveDesc m_PrimDesc;

	//	// stores the original position and normal
	//	struct oVertex {
	//		oVertex() = default;
	//		glm::vec3 position;
	//		glm::vec3 normal;
	//	} m_ov0, m_ov1, m_ov2;
	//};
}
