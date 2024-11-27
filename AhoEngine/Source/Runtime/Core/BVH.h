#pragma once

#include "Runtime/Core/Math/Math.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "AABB.h"

namespace Aho {
	struct Ray {
		Ray(const glm::vec3& origin, const glm::vec3& direction)
			: origin(origin), direction(direction) { }
		glm::vec3 origin, direction;
	};

	struct Primitive {
		Primitive() = default;
		Primitive(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2, int id)
			: p0(p0), p1(p1), p2(p2), uv0(uv0), uv1(uv1), uv2(uv2), meshID(id) { }

		Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
			p0 = glm::vec3(v0.x, v0.y, v0.z);
			p1 = glm::vec3(v1.x, v1.y, v1.z);
			p2 = glm::vec3(v2.x, v2.y, v2.z);
		
			
		}

		AABB ComputeAABB() const {
			glm::vec3 minPoint = glm::min(glm::min(p0, p1), p2);
			glm::vec3 maxPoint = glm::max(glm::max(p0, p1), p2);
			return AABB(minPoint, maxPoint);
		}

		//int id;
		glm::vec3 p0, p1, p2;
		glm::vec2 uv0, uv1, uv2;
		int meshID;
	};

	struct IntersectResult {
		IntersectResult(float t, const glm::vec3 pos, const glm::vec3& normal, const glm::vec3& baryC, const glm::vec2& uv) 
			: t(t), position(pos), normal(normal), baryCentric(baryC), uv(uv) {}
		float t;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 baryCentric;
		glm::vec2 uv;
	};

	struct BVHNode {
		BVHNode() {
			aabb = AABB();
			left = right = nullptr;
		}
		AABB aabb;
		std::vector<Primitive> primitives;
		BVHNode* left;
		BVHNode* right;
		int axis{ 0 };
	};

	class BVH {
	public:
		enum class SplitMethod {
			NAIVE,
			SAH
		};

	public:
		BVH(const std::shared_ptr<StaticMesh>& mesh, int primsPerNode = 1, SplitMethod method = SplitMethod::NAIVE)
			: m_PrimsPerNode(primsPerNode), m_Primitive(primsPerNode), m_SplitMethod(method) {
			Build(mesh);
		}
			
	private:
		void Build(const std::shared_ptr<StaticMesh>& mesh) {
			// Assume triangle
			const auto& meshInfo = mesh->GetMeshInfo();
			for (const auto& info : meshInfo) {
				const auto& vertices = info->vertexBuffer;
				const auto& indices = info->indexBuffer;
				size_t siz = indices.size();
				AHO_CORE_ASSERT(siz % 3 == 0);
				for (size_t i = 0; i < siz; i += 3) {
					const Vertex& v0 = vertices[indices[i]];
					const Vertex& v1 = vertices[indices[i + 1]];
					const Vertex& v2 = vertices[indices[i + 1]];

					Primitive p(v0, v1, v2);
				}
			}

		}

	private:
		const int m_PrimsPerNode;
		const SplitMethod m_SplitMethod;
		std::vector<Primitive> m_Primitive;
	};
}
