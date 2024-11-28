#pragma once

#include "Runtime/Core/Math/Math.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "AABB.h"

namespace Aho {
	struct Ray {
		Ray() = default;
		Ray(const glm::vec3& origin, const glm::vec3& direction)
			: origin(origin), direction(direction) { }
		glm::vec3 origin, direction;
	};

	class Primitive {
	public:
		Primitive() = default;

		Primitive(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: m_Dirty(true), m_v0(v0), m_v1(v1), m_v2(v2) { }

		AABB GetAABB() {
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

		glm::vec3 GetCentroid() {
			return GetAABB().GetCentroid();
		}

		Vertex GetVertex(int num) const {
			return num == 0 ? m_v0
				: num == 1 ? m_v1
				: m_v2;
		}

	private:
		//int id;
		AABB m_AABB;
		bool m_Dirty;
		Vertex m_v0;
		Vertex m_v1;
		Vertex m_v2;
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
		std::vector<Primitive*> primitives;
		//Primitive primitive;
		BVHNode* left;
		BVHNode* right;
		Axis axis{ Axis::X };
	};

	class BVH {
	public:
		enum class SplitMethod {
			NAIVE,
			SAH
		};

	public:
		BVH(const std::shared_ptr<StaticMesh>& mesh, int primsPerNode = 1, SplitMethod method = SplitMethod::NAIVE)
			: m_Root(nullptr), m_PrimsPerNode(primsPerNode), m_SplitMethod(method) {
			Build(mesh);
		}

		BVHNode* GetRoot() {
			return m_Root;
		}

		static std::optional<IntersectResult> GetIntersection(Ray ray, BVHNode* node) {
			return Traverse(ray, node);
		}

	private:
		static std::optional<IntersectResult> Traverse(const Ray ray, const BVHNode* node) {
			//AHO_CORE_ASSERT(node);
			if (!node) {
				return std::nullopt;
			}

			if (!Intersect(ray, node->aabb)) {
				return std::nullopt;
			}
			if (!node->primitives.empty()) {
				const Primitive* p = node->primitives[0];
				auto res = Intersect(ray, p);
				if (res) {
					return res;
				}
			}
			auto result = Traverse(ray, node->left);
			return result ? result : Traverse(ray, node->right);
		}

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
					const Vertex& v2 = vertices[indices[i + 2]];
					auto p = new Primitive(v0, v1, v2);
					m_Primitive.push_back(p);
				}
			}
			m_Root = BuildTree(m_Primitive);
			AHO_CORE_WARN("Total build: {}, {}", cnt, m_Primitive.size());
		}

		int cnt = 0;
		BVHNode* BuildTree(std::vector<Primitive*> primitives) {
			BVHNode* node = new BVHNode();

			AABB aabb;
			for (Primitive* p : primitives) {
				AHO_CORE_ASSERT(p);
				aabb.Expand(p->GetAABB());
			}
			node->aabb = aabb;
			
			if (primitives.size() == 1) {
				cnt += 1;
				//AHO_CORE_WARN("Case 1");
				node->primitives = { primitives[0] };
			}
			else if (primitives.size() == 2) {
				//cnt += 2;
				//AHO_CORE_WARN("Case 2");
				node->left = BuildTree({ primitives[0] });
				node->right = BuildTree({ primitives[1] });
			}
			else {
				int axis = (int)aabb.GetSplitAxis();
				//AHO_CORE_WARN("Case 3, axis : {}", axis);

				std::sort(primitives.begin(), primitives.end(), [axis](Primitive* lhs, Primitive* rhs) {
					return lhs->GetCentroid()[axis] < rhs->GetCentroid()[axis];
				});

				auto Lvec = std::vector<Primitive*>(primitives.begin(), primitives.begin() + primitives.size() / 2);
				auto Rvec = std::vector<Primitive*>(primitives.begin() + primitives.size() / 2, primitives.end());
				AHO_CORE_ASSERT(Lvec.size() + Rvec.size() == primitives.size());
				AHO_CORE_WARN("{}, {}", Lvec.size(), Rvec.size());
				node->left = BuildTree(Lvec);
				node->right = BuildTree(Rvec);
			}

			return node;
		}

	private:
		BVHNode* m_Root;
		const int m_PrimsPerNode;
		const SplitMethod m_SplitMethod;
		std::vector<Primitive*> m_Primitive;
	};
}
