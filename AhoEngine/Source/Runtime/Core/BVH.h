#pragma once

#include "Runtime/Function/Renderer/Material.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "Primitive.h"
#include "BBox.h"
#include "Runtime/Core/Math/Math.h"

#include <stack>
#include <variant>

namespace Aho {
	
	// Some general bvh props
	enum class SplitMethod {
		NAIVE,
		SAH
	};

	enum class BVHLevel {
		BLAS,
		TLAS
	};

	// Heuristic number from pbrt https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
	constexpr int SAH_SPLIT_BUCKETS_NUM = 12;

	// The maximum number of primitives contained in a leaf node
	constexpr int LEAF_PRIMS = 1;

	struct alignas(16) BVHNodei {
		BVHNodei() : left(-1), right(-1), nodeIdx(-1), firstPrimsIdx(-1), primsCnt(-1), axis(-1) {}
		void InitLeaf(int nodeIdx_, int firstPrimsIdx_, int primsCnt_, const BBox& bbox_) {
			//AHO_CORE_ASSERT(primsCnt_ > 0 && primsCnt_ <= LEAF_PRIMS);
			//if (primsCnt_ > 10) {
			//	AHO_CORE_INFO("Initializing leaf with `{}` primitives", primsCnt_);
			//}
			nodeIdx = nodeIdx_;
			left = -1;
			right = -1;
			firstPrimsIdx = firstPrimsIdx_;
			primsCnt = primsCnt_;
			bbox = bbox_;
		}

		void InitInterior(int nodeIdx_, int l, int r, const BBox& bbox_, int axis_) {
			nodeIdx = nodeIdx_;
			firstPrimsIdx = -1;
			primsCnt = 0;
			left = l, right = r;
			axis = axis_;
			bbox = bbox_;
		}

		bool IsLeaf() const {
			return primsCnt > 0;
		}

		BBox GetBBox() const { return bbox; }

		BBox bbox;
		int left, right;
		int nodeIdx;
		int firstPrimsIdx;
		int primsCnt;
		int axis;
		int meshId;
		int offset;
	};

	struct alignas(16) OffsetInfo {
		OffsetInfo() = default;
		OffsetInfo(int id, int node, int prim) 
			: offset(id), nodeOffset(node), primOffset(prim) {
		}
		int _{ -1 };
		int offset;
		int nodeOffset;
		int primOffset;
	};

	class BVHi {
	public:
		BVHi() 
			: m_SplitMethod(SplitMethod::NAIVE), m_BvhLevel(BVHLevel::TLAS) {
		}
		
		BVHi(const std::shared_ptr<StaticMesh>& mesh, SplitMethod splitMethod = SplitMethod::SAH) 
			: m_SplitMethod(splitMethod), m_BvhLevel(BVHLevel::BLAS) {
			AHO_CORE_ASSERT(false);
			Build(mesh);
		}
		
		BVHi(const std::shared_ptr<MeshInfo>& info, int meshId, MaterialMaskEnum mask = MaterialMaskEnum::Empty, SplitMethod splitMethod = SplitMethod::SAH)
			: m_MeshId(meshId), m_MaterialMask(mask), m_SplitMethod(splitMethod), m_BvhLevel(BVHLevel::BLAS) {
			Build(info);
		}

		size_t GetNodeCount() const { return m_Nodes.size(); }
		size_t GetPrimsCount() const { return m_Primitives.size(); }

		bool Intersect(const Ray& ray);

		void ApplyTransform(const glm::mat4& transform);

		void UpdateTLAS();

		int GetRoot() const {
			AHO_CORE_ASSERT(m_Root == 0);
			return m_Root;
		}

		int GetMeshId() const { return m_MeshId; }

		BBox GetBBox() const {
			AHO_CORE_ASSERT(m_Root == 0);
			return m_Nodes[m_Root].GetBBox();
		}

		void AddBLASPrimtive(const BVHi* blas);

		const std::vector<BVHNodei>& GetNodesArr() const { return m_Nodes; }

		const std::vector<PrimitiveDesc>& GetPrimsArr() const { return m_Primitives; }

		const std::vector<OffsetInfo>& GetOffsetMap() const {
			AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS);
			return m_OffsetMap;
		}

		const BVHi* GetBLAS(int id) const {
			AHO_CORE_ASSERT(id >= 0 && id < m_BLAS.size());
			AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS);
			return m_BLAS[id];
		}
	private:
		bool IntersectNearest_recursion(const Ray& ray, int root);
		bool IntersectNearest_loop(const Ray& ray, float& t);
		void Build(const std::shared_ptr<StaticMesh>& mesh);
		void Build(const std::shared_ptr<MeshInfo>& mesh);

		// Build tree for primitives of intervals [indexL, indexR)
		int BuildTreeRecursion(int indexL, int indexR);

	private:
		inline static size_t s_NodeOffset{ 0 };
		inline static size_t s_PrimOffset{ 0 };

	private:
		MaterialMaskEnum m_MaterialMask{ MaterialMaskEnum::Empty };
		int m_MeshId{ -1 };
		int m_Root{ -1 };
		BVHLevel m_BvhLevel;
		SplitMethod m_SplitMethod; // const member var will delete default operator=

	private:
		std::vector<BVHi*> m_BLAS;
		std::vector<OffsetInfo> m_OffsetMap;
		std::vector<PrimitiveDesc> m_Primitives;
		std::vector<PrimitiveCompliment> m_PrimitiveComp;
		std::vector<BVHNodei> m_Nodes; // dfs order
	};


	//template<typename PrimitiveType>
	//inline bool BVHi<PrimitiveType>::Intersect(const Ray& ray) {
	//	//return IntersectRecursion(ray, m_Root);
	//	return IntersectLoop(ray);
	//}

	//template<typename PrimitiveType>
	//inline void BVHi<PrimitiveType>::ApplyTransform(const glm::mat4& transform) {
	//	for (auto& primitive : m_Primitives) {
	//		primitive.ApplyTransform(transform);
	//	}
	//	m_Nodes.clear();
	//	m_Root = BuildTreeRecursion(0, m_Primitives.size());
	//}

	//template<typename PrimitiveType>
	//bool BVHi<PrimitiveType>::IntersectRecursion(const Ray& ray, int root) {
	//	AHO_CORE_ASSERT(root >= 0 && root < m_Nodes.size());
	//	const BVHNodei& node = m_Nodes[root];
	//	if (!node.bbox.Intersect(ray)) {
	//		return false;
	//	}

	//	if (node.IsLeaf()) {
	//		for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
	//			AHO_CORE_ASSERT(i < m_Primitives.size());
	//			const Primitive& p = m_Primitives[i];
	//			if (p.Intersect(ray)) {
	//				return true;
	//			}
	//		}
	//		return false;
	//	}
	//	return IntersectRecursion(ray, node.left) ? true : IntersectRecursion(ray, node.right);
	//}

	//// Traverse bvh without recursion
	//template<typename PrimitiveType>
	//inline bool BVHi<PrimitiveType>::IntersectLoop(const Ray& ray) {
	//	int n = m_Nodes.size();
	//	std::stack<int> stk;
	//	stk.push(m_Root);

	//	int find = -1;
	//	while (!stk.empty()) {
	//		int u = stk.top();
	//		stk.pop();

	//		const BVHNodei& node = m_Nodes[u];
	//		if (!node.bbox.Intersect(ray)) {
	//			continue;
	//		}

	//		if (node.IsLeaf()) {
	//			for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
	//				AHO_CORE_ASSERT(i < m_Primitives.size());
	//				const Primitive& p = m_Primitives[i];
	//				if (p.Intersect(ray)) {
	//					find = u;
	//					break;
	//				}
	//			}
	//			if (find != -1) {
	//				break;
	//			}	
	//		}
	//		else {
	//			stk.push(node.right);
	//			stk.push(node.left);
	//		}
	//	}

	//	return find != -1;
	//}

	//template<typename PrimitiveType>
	//inline void BVHi<PrimitiveType>::Build(const std::shared_ptr<StaticMesh>& mesh) {
	//	m_Primitives.reserve(mesh->GetVerticesCount() / 3);
	//	const auto& meshInfo = mesh->GetMeshInfo();
	//	for (const auto& info : meshInfo) {
	//		const auto& vertices = info->vertexBuffer;
	//		const auto& indices = info->indexBuffer;
	//		size_t siz = indices.size();
	//		AHO_CORE_ASSERT(siz % 3 == 0);
	//		for (size_t i = 0; i < siz; i += 3) {
	//			const Vertex& v0 = vertices[indices[i]];
	//			const Vertex& v1 = vertices[indices[i + 1]];
	//			const Vertex& v2 = vertices[indices[i + 2]];
	//			m_Primitives.emplace_back(v0, v1, v2);
	//		} 
	//	}

	//	m_Nodes.reserve(m_Primitives.size());
	//	m_Root = BuildTreeRecursion(0, m_Primitives.size());
	//	m_Nodes.shrink_to_fit();
	//}

	//template<typename PrimitiveType>
	//void BVHi<PrimitiveType>::Build(const std::shared_ptr<MeshInfo>& info) {
	//	const auto& vertices = info->vertexBuffer;
	//	const auto& indices = info->indexBuffer;
	//	size_t siz = indices.size();
	//	AHO_CORE_ASSERT(siz % 3 == 0);

	//	m_Primitives.reserve(siz);
	//	for (size_t i = 0; i < siz; i += 3) {
	//		const Vertex& v0 = vertices[indices[i]];
	//		const Vertex& v1 = vertices[indices[i + 1]];
	//		const Vertex& v2 = vertices[indices[i + 2]];
	//		m_Primitives.emplace_back(v0, v1, v2);
	//	}

	//	m_Nodes.reserve(m_Primitives.size());
	//	m_Root = BuildTreeRecursion(0, m_Primitives.size());
	//	m_Nodes.shrink_to_fit();
	//}

	//template<typename PrimitiveType>
	//void BVHi<PrimitiveType>::Build(const std::vector<BVHi>& sceneBvhs) {
	//	m_Nodes.reserve(sceneBvhs.size());

	//	auto build =
	//		[&](auto&& self, int l, int r) -> int {
	//		int nodeIndex = m_Nodes.size();
	//		m_Nodes.push_back(BVHNodei());

	//		};

	//	m_Root = build(build, 0, m_Nodes.size());

	//}

	//template<typename PrimitiveType>
	//int BVHi<PrimitiveType>::BuildTreeRecursion(int indexL, int indexR) {
	//	AHO_CORE_ASSERT(indexL < indexR);
	//	int nodeIndex = m_Nodes.size();

	//	m_Nodes.push_back(BVHNodei());
	//	//BVHNodei& node = m_Nodes.back(); null reference!

	//	AABB aabb, centroid;
	//	for (int i = indexL; i < indexR; i++) {
	//		const auto& p = m_Primitives[i];
	//		aabb.Merge(p.GetAABB());
	//		centroid.Merge(p.GetCentroid());
	//	}

	//	int primsCnt = indexR - indexL;
	//	if (primsCnt <= LEAF_PRIMS) {
	//		m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, aabb);
	//	}
	//	else {
	//		int axis = (int)aabb.GetSplitAxis();

	//		switch (m_SplitMethod) {
	//			case (SplitMethod::NAIVE): {

	//				std::sort(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
	//					[axis](const Primitive& lhs, const Primitive& rhs) {
	//						return lhs.GetCentroid()[axis] < rhs.GetCentroid()[axis];
	//					});

	//				int l = BuildTreeRecursion(indexL, indexL + primsCnt / 2);
	//				int r = BuildTreeRecursion(indexL + primsCnt / 2, indexR);

	//				m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, aabb, axis);
	//				break;
	//			}

	//			case (SplitMethod::SAH): {

	//				std::array<int, SAH_SPLIT_BUCKETS_NUM> bucketsCount{};
	//				std::array<AABB, SAH_SPLIT_BUCKETS_NUM> bucketsBBox{};

	//				for (int i = indexL; i < indexR; i++) {
	//					const auto& p = m_Primitives[i];
	//					int b = SAH_SPLIT_BUCKETS_NUM *
	//						centroid.Offset(p.GetCentroid())[axis];

	//					if (b == SAH_SPLIT_BUCKETS_NUM) {
	//						b -= 1;
	//					}

	//					bucketsCount[b] += 1;
	//					bucketsBBox[b].Merge(p.GetAABB());
	//				}

	//				std::array<AABB, 1 + SAH_SPLIT_BUCKETS_NUM> suffixBBox{};
	//				for (int i = SAH_SPLIT_BUCKETS_NUM - 1; i > 0; i--) {
	//					suffixBBox[i].Merge(suffixBBox[i + 1]);
	//					suffixBBox[i].Merge(bucketsBBox[i]);
	//				}

	//				AABB prevBBox;
	//				int totBBox = primsCnt;
	//				int prevBBoxCnt = 0;
	//				std::array<float, SAH_SPLIT_BUCKETS_NUM> costs{};
	//				std::fill(costs.begin(), costs.end(), std::numeric_limits<float>::max());
	//				int splitPos = 0;
	//				// split into two sets: [0, i], [i + 1, SAH_SPLIT_BUCKETS_NUM - 1]
	//				for (int i = 0; i < SAH_SPLIT_BUCKETS_NUM - 1; i++) {
	//					prevBBox.Merge(bucketsBBox[i]);
	//					prevBBoxCnt += bucketsCount[i];

	//					costs[i] = 1.0f + (prevBBox.GetSurfaceArea() * prevBBoxCnt
	//						+ suffixBBox[i + 1].GetSurfaceArea() * (totBBox - prevBBoxCnt)) / aabb.GetSurfaceArea();
	//					if (costs[i] < costs[splitPos]) {
	//						splitPos = i;
	//					}
	//				}

	//				float leafCost = primsCnt;
	//				if (costs.at(splitPos) < leafCost) {
	//					int mid = std::partition(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
	//						[&](const Primitive& p) {
	//							int b = SAH_SPLIT_BUCKETS_NUM *
	//								centroid.Offset(p.GetCentroid())[axis];

	//							if (b == SAH_SPLIT_BUCKETS_NUM) {
	//								b -= 1;
	//							}

	//							return b <= splitPos;
	//						}) - m_Primitives.begin();

	//					int l = BuildTreeRecursion(indexL, mid);
	//					int r = BuildTreeRecursion(mid, indexR);

	//					m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, aabb, axis);
	//				}
	//				else {
	//					// create leaf directly, ignoring LEAF_PRIMS
	//					m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, aabb);
	//				}

	//				break;
	//			}
	//		}
	//	}

	//	return nodeIndex;
	//}

}


// Trash code 
//// Pointer Based Bvh
//struct BVHNode {
//	BVHNode() = default;
//	AABB aabb;
//	std::vector<Primitive*> primitives;
//	std::unique_ptr<BVHNode> left{ nullptr };
//	std::unique_ptr<BVHNode> right{ nullptr };
//	Axis axis{ Axis::X };
//};
//
//class BVH {
//public:
//	BVH() : m_SplitMethod(SplitMethod::NAIVE) {}
//	std::unique_ptr<BVHNode>& GetRoot() { return m_Root; }
//	static std::optional<IntersectResult> GetIntersection(const Ray& ray, BVHNode* node) { return GetIntersectionRecursion(ray, node); }
//	const std::unique_ptr<BVHNode>& AddPrimitives(std::vector<std::unique_ptr<Primitive>>& primitives);
//	static std::vector<std::unique_ptr<Primitive>> GeneratePrimitives(const std::shared_ptr<StaticMesh>& mesh, const std::unordered_map<std::string, std::shared_ptr<Texture2D>>& textureCached);
//	void BuildTree();
//private:
//	static std::optional<IntersectResult> GetIntersectionRecursion(const Ray& ray, BVHNode* node);
//
//	// [indexL, indexR)
//	std::unique_ptr<BVHNode> BuildTreeRecursion(int indexL, int indexR);
//private:
//	//const int m_PrimsPerNode; do we need this?
//	std::unique_ptr<BVHNode> m_Root{ nullptr };
//	const SplitMethod m_SplitMethod;
//	std::vector<std::unique_ptr<Primitive>> m_Primitives;
//};
//
//