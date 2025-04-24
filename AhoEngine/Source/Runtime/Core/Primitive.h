#pragma once

#include "Runtime/Function/Renderer/Material.h"
#include "Runtime/Core/Math/Math.h"
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
		PrimitiveDesc(const Vertex& v0, const Vertex& v1, const Vertex& v2, int id, MaterialMaskEnum mask = MaterialMaskEnum::Empty, int meshId = -1)
			: m_PrimId(id), m_MaterialMask(mask), m_MeshId(meshId) {
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
		int GetId() const { return m_Id; } 
		void SetID(int id) { m_Id = id; }
		int GetPrimId() const { return m_PrimId; } // this id is for transform only
		void SetMaterialMask(MaterialMaskEnum mask) { m_MaterialMask = mask; }

	private:
		BBox m_BBox;
		Vertex m_Vertices[3];
		int m_MeshId;		// which submesh it belongs to. MeshId is a global id
		int m_Id{ -1 };		// In tlas bvh, 
		int m_PrimId{ -1 }; // In bvh primtives array(both blas, tlas), the order would change during tree construction. This is the origin order of this primitive
		MaterialMaskEnum m_MaterialMask{ MaterialMaskEnum::Empty };
		
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

			tangent[0] = v0.tangent;
			tangent[1] = v1.tangent;
			tangent[2] = v2.tangent;
		}
		glm::vec3 position[3];
		glm::vec3 normal[3];
		glm::vec3 tangent[3];
	};
}
