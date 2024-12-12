#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "Primitive.h"
#include "AABB.h"

#include <variant>

namespace Aho {
	struct BVHNode {
		BVHNode() = default;
		AABB aabb;
		std::vector<Primitive*> primitives;
		std::unique_ptr<BVHNode> left{ nullptr };
		std::unique_ptr<BVHNode> right{ nullptr };
		Axis axis{ Axis::X };
	};

	class BVH {
	public:
		enum class SplitMethod {
			NAIVE,
			SAH
		};

	public:
		BVH() : m_SplitMethod(SplitMethod::NAIVE) {}
		std::unique_ptr<BVHNode>& GetRoot() { return m_Root; }
		static std::optional<IntersectResult> GetIntersection(const Ray& ray, BVHNode* node) { return GetIntersectionRecursion(ray, node); }
		const std::unique_ptr<BVHNode>& AddPrimitives(std::vector<std::unique_ptr<Primitive>>& primitives);
		static std::vector<std::unique_ptr<Primitive>> GeneratePrimitives(const std::shared_ptr<StaticMesh>& mesh, const std::unordered_map<std::string, std::shared_ptr<Texture2D>>& textureCached);
		void BuildTree();
	private:
		static std::optional<IntersectResult> GetIntersectionRecursion(const Ray& ray, BVHNode* node);

		// [indexL, indexR)
		std::unique_ptr<BVHNode> BuildTreeRecursion(int indexL, int indexR);
	private:
		//const int m_PrimsPerNode; do we need this?
		std::unique_ptr<BVHNode> m_Root{ nullptr };
		const SplitMethod m_SplitMethod;
		std::vector<std::unique_ptr<Primitive>> m_Primitives;
	};
}
