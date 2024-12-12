#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "AABB.h"

namespace Aho {
	struct PrimitiveMaterial {
		PrimitiveMaterial() = default;
		PrimitiveMaterial(const glm::vec4& albedo, const glm::vec4& normal, const glm::vec4& pbr)
			: m_Albedo(albedo), m_Normal(normal), m_PBR(pbr) { }
		glm::vec4 m_Albedo{ 0.95f, 0.95f, 0.95f, 1.0f };
		glm::vec4 m_Normal{ 0.0f, 1.0f, 0.0f, 0.0f };
		glm::vec4 m_PBR{ 0.9f, 0.2f, 0.0f, 0.0f }; // Roughness, Metalic
	};

	class Primitive {
	public:

	public:
		Primitive() = default;

		Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2, const PrimitiveMaterial& mat0, const PrimitiveMaterial& mat1, const PrimitiveMaterial& mat2)
			: m_Dirty(true), m_v0(v0), m_v1(v1), m_v2(v2), m_Material0(mat0), m_Material1(mat1), m_Material2(mat2) {
		}

		AABB GetAABB();
		void SetTransform(TransformParam* transform) { m_Transform = transform; }
		void Update();
		glm::vec3 GetCentroid() { return GetAABB().GetCentroid(); }
		Vertex GetVertex(int num) const { return num == 0 ? m_v0 : num == 1 ? m_v1 : m_v2; }

		PrimitiveMaterial GetMaterial() { return m_Material0; }

	private:
		//int id;
		bool m_Dirty;
		TransformParam* m_Transform{ nullptr };
		AABB m_AABB;
		Vertex m_v0;
		Vertex m_v1;
		Vertex m_v2;
		PrimitiveMaterial m_Material0;
		PrimitiveMaterial m_Material1;
		PrimitiveMaterial m_Material2;
		int meshID{ -1 };
	};
}
