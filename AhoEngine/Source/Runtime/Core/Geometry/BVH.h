#pragma once

#include "Runtime/Core/Core.h"
#include "BVHNode.h"
#include "Primitive.h"
#include "Runtime/Core/Math/Math.h"

#include <stack>
#include <variant>

namespace Aho {
	class StaticMesh;
	class MeshInfo;
	struct Mesh;
	class BVHi {
	public:
		BVHi()
			: m_SplitMethod(SplitMethod::NAIVE), m_BvhLevel(BVHLevel::TLAS) {
		}
		BVHi(const Mesh& mesh, int meshId, SplitMethod splitMethod = SplitMethod::SAH)
			: m_MeshId(meshId), m_SplitMethod(splitMethod), m_BvhLevel(BVHLevel::BLAS) {
			Build(mesh);
		}
		BVHi(const std::shared_ptr<MeshInfo>& info, int meshId, SplitMethod splitMethod = SplitMethod::SAH)
			: m_MeshId(meshId), m_SplitMethod(splitMethod), m_BvhLevel(BVHLevel::BLAS) {
			Build(info);
		}
		size_t GetNodeCount() const { return m_Nodes.size(); }
		size_t GetPrimsCount() const { return m_Primitives.size(); }
		bool Intersect(const Ray& ray);
		void ApplyTransform(const glm::mat4& transform);
		void Rebuild();
		void UpdateTLAS();
		int GetRoot() const { AHO_CORE_ASSERT(m_Root == 0); return m_Root; }
		int GetMeshId() const { return m_MeshId; }
		BBox GetBBox() const { AHO_CORE_ASSERT(m_Root == 0); return m_Nodes[m_Root].GetBBox(); }
		void AddBLASPrimtive(const BVHi* blas);
		const std::vector<BVHNodei>& GetNodesArr() const { return m_Nodes; }
		const std::vector<PrimitiveDesc>& GetPrimsArr() const { return m_Primitives; }
		const std::vector<OffsetInfo>& GetOffsetMap() const { AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS); return m_OffsetMap; }
		const BVHi* GetBLAS(int id) const;
	private:
		bool IntersectNearest_recursion(const Ray& ray, int root);
		bool IntersectNearest_loop(const Ray& ray, float& t);
		void Build(const Mesh& mesh);
		void Build(const std::shared_ptr<MeshInfo>& mesh);
		// Build tree for primitives of intervals [indexL, indexR)
		int BuildTreeRecursion(int indexL, int indexR);
	private:
		inline static size_t s_NodeOffset{ 0 };
		inline static size_t s_PrimOffset{ 0 };
	private:
		int m_MeshId{ -1 }; // Global mesh id
		int m_Root{ -1 };
		BVHLevel m_BvhLevel;
		SplitMethod m_SplitMethod; // const member var will delete default operator=
	private:
		std::vector<BVHi*> m_BLAS;
		std::vector<OffsetInfo> m_OffsetMap; // Every blas as a primitive has a corresponding offsetInfo; BLAS primtives are stored in a big array in shader, so for every mesh we need this to spicify its starting primtives id
		std::vector<PrimitiveDesc> m_Primitives;
		std::vector<PrimitiveCompliment> m_PrimitiveComp;
		std::vector<BVHNodei> m_Nodes; // dfs order
	};
}