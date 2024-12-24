#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "Primitive.h"
#include "AABB.h"
#include "Runtime/Core/Math/Math.h"

#include <stack>
#include <variant>

namespace Aho {
	
	// Some general bvh props
	enum class SplitMethod {
		NAIVE,
		SAH
	};

	// Magic number from pbrt https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
	constexpr int SAH_SPLIT_BUCKETS_NUM = 12;

	// The maximum number of primitives contained in a leaf node
	constexpr int LEAF_PRIMS = 4;


	// Pointer Based Bvh
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


	// Index based Bvh, useful when passing to gpu
	using Bbox = AABB;
	struct BVHNodei {
		BVHNodei() : left(-1), right(-1), nodeIdx(-1), firstPrimsIdx(-1), primsCnt(-1), axis(-1) {}
		void InitLeaf(int nodeIdx_, int firstPrimsIdx_, int primsCnt_, const Bbox& bbox_) {
			AHO_CORE_ASSERT(primsCnt_ > 0 && primsCnt_ <= LEAF_PRIMS);
			nodeIdx = nodeIdx_;
			left = right = -1;
			firstPrimsIdx = firstPrimsIdx_;
			primsCnt = primsCnt_;
			bbox = bbox_;
			axis = 0;
		}
	
		void InitInterior(int nodeIdx_, int l, int r, const Bbox& bbox_, int axis_) {
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

		Bbox GetBbox() const { return bbox; }

		Bbox bbox;
		int left, right;
		int nodeIdx;
		int firstPrimsIdx;
		int primsCnt;
		int axis;
	};


	template <typename T>
	class BVHi {
	public:
		template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, Primitive>>>
		BVHi(const std::shared_ptr<StaticMesh>& mesh, SplitMethod splitMethod = SplitMethod::SAH) 
			: m_SplitMethod(splitMethod) {
			Build(mesh);
		}

		template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, BVHi<Primitive>>>>
		BVHi(const std::vector<BVHi>& sceneBvhs, SplitMethod splitMethod = SplitMethod::NAIVE)
			: m_SplitMethod(splitMethod) {
			Build(sceneBvhs);
		}

		int GetIntersectNode(const Ray& ray);

		bool Intersect(const Ray& ray);

		void ApplyTransform(const glm::mat4& transform);

		int GetRoot() const { return m_Root; }
		Bbox GetBbox() const {
			AHO_CORE_ASSERT(m_Root >= 0 && m_Root < m_Nodes.size()); 
			return m_Nodes[m_Root].GetBbox(); 
		}

		// Get the bounding box of a specific node
		Bbox GetNodeBbox(int index) const {
			AHO_CORE_ASSERT(index >= 0 && index < m_Nodes.size());
			return m_Nodes[index].GetBbox();
		}

	private:
		bool IntersectRecursion(const Ray& ray, int root);
		bool IntersectLoop(const Ray& ray);
		void Build(const std::shared_ptr<StaticMesh>& mesh);
		void Build(const std::vector<BVHi>& sceneBvhs);
		
		// Build tree for primitives of intervals [indexL, indexR)
		int BuildTreeRecursion(int indexL, int indexR);
	private:
		int m_Root{ -1 };
		std::vector<T> m_Primitives;
		const SplitMethod m_SplitMethod;
		std::vector<BVHNodei> m_Nodes; // dfs order
	};


	template<typename T>
	int BVHi<T>::GetIntersectNode(const Ray& ray) {
		return 0;
	}

	template<typename T>
	inline bool BVHi<T>::Intersect(const Ray& ray) {
		//return IntersectRecursion(ray, m_Root);
		return IntersectLoop(ray);
	}

	template<typename T>
	inline void BVHi<T>::ApplyTransform(const glm::mat4& transform) {
		for (auto& primitive : m_Primitives) {
			primitive.ApplyTransform(transform);
		}
		m_Nodes.clear();
		m_Root = BuildTreeRecursion(0, m_Primitives.size());
	}

	template<typename T>
	bool BVHi<T>::IntersectRecursion(const Ray& ray, int root) {
		AHO_CORE_ASSERT(root >= 0 && root < m_Nodes.size());
		const BVHNodei& node = m_Nodes[root];
		if (!node.bbox.Intersect(ray)) {
			return false;
		}

		if (node.IsLeaf()) {
			for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
				AHO_CORE_ASSERT(i < m_Primitives.size());
				const Primitive& p = m_Primitives[i];
				if (p.Intersect(ray)) {
					return true;
				}
			}
			return false;
		}
		return IntersectRecursion(ray, node.left) ? true : IntersectRecursion(ray, node.right);
	}

	// Traverse bvh without recursion
	template<typename T>
	inline bool BVHi<T>::IntersectLoop(const Ray& ray) {
		int n = m_Nodes.size();
		std::stack<int> stk;
		stk.push(m_Root);

		int find = -1;
		while (!stk.empty()) {
			int u = stk.top();
			stk.pop();

			const BVHNodei& node = m_Nodes[u];
			if (!node.bbox.Intersect(ray)) {
				continue;
			}

			if (node.IsLeaf()) {
				for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
					AHO_CORE_ASSERT(i < m_Primitives.size());
					const Primitive& p = m_Primitives[i];
					if (p.Intersect(ray)) {
						find = u;
						break;
					}
				}
				if (find != -1) {
					break;
				}	
			}
			else {
				stk.push(node.right);
				stk.push(node.left);
			}
		}

		return find != -1;
	}

	template<typename T>
	inline void BVHi<T>::Build(const std::shared_ptr<StaticMesh>& mesh) {
		m_Primitives.reserve(mesh->GetVerticesCount() / 3);
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
				m_Primitives.emplace_back(v0, v1, v2);
			} 
		}

		m_Nodes.reserve(m_Primitives.size());
		m_Root = BuildTreeRecursion(0, m_Primitives.size());
		m_Nodes.shrink_to_fit();
	}

	template<typename T>
	void BVHi<T>::Build(const std::vector<BVHi>& sceneBvhs) {

	
	}

	template<typename T>
	int BVHi<T>::BuildTreeRecursion(int indexL, int indexR) {
		AHO_CORE_ASSERT(indexL < indexR);
		int nodeIndex = m_Nodes.size();

		m_Nodes.push_back(BVHNodei());
		//BVHNodei& node = m_Nodes.back(); null reference!

		AABB aabb, centroid;
		for (int i = indexL; i < indexR; i++) {
			const auto& p = m_Primitives[i];
			aabb.Merge(p.GetAABB());
			centroid.Merge(p.GetCentroid());
		}

		int primsCnt = indexR - indexL;
		if (primsCnt <= LEAF_PRIMS) {
			m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, aabb);
		}
		else {
			int axis = (int)aabb.GetSplitAxis();

			switch (m_SplitMethod) {
				case (SplitMethod::NAIVE): {

					std::sort(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
						[axis](const Primitive& lhs, const Primitive& rhs) {
							return lhs.GetCentroid()[axis] < rhs.GetCentroid()[axis];
						});

					int l = BuildTreeRecursion(indexL, indexL + primsCnt / 2);
					int r = BuildTreeRecursion(indexL + primsCnt / 2, indexR);

					m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, aabb, axis);
					break;
				}

				case (SplitMethod::SAH): {

					std::array<int, SAH_SPLIT_BUCKETS_NUM> bucketsCount{};
					std::array<AABB, SAH_SPLIT_BUCKETS_NUM> bucketsBbox{};

					for (int i = indexL; i < indexR; i++) {
						const auto& p = m_Primitives[i];
						int b = SAH_SPLIT_BUCKETS_NUM *
							centroid.Offset(p.GetCentroid())[axis];

						if (b == SAH_SPLIT_BUCKETS_NUM) {
							b -= 1;
						}

						bucketsCount[b] += 1;
						bucketsBbox[b].Merge(p.GetAABB());
					}

					std::array<AABB, 1 + SAH_SPLIT_BUCKETS_NUM> suffixBbox{};
					for (int i = SAH_SPLIT_BUCKETS_NUM - 1; i > 0; i--) {
						suffixBbox[i].Merge(suffixBbox[i + 1]);
						suffixBbox[i].Merge(bucketsBbox[i]);
					}

					AABB prevBbox;
					int totBbox = primsCnt;
					int prevBboxCnt = 0;
					std::array<float, SAH_SPLIT_BUCKETS_NUM> costs{};
					std::fill(costs.begin(), costs.end(), std::numeric_limits<float>::max());
					int splitPos = 0;
					// split into two sets: [0, i], [i + 1, SAH_SPLIT_BUCKETS_NUM - 1]
					for (int i = 0; i < SAH_SPLIT_BUCKETS_NUM - 1; i++) {
						prevBbox.Merge(bucketsBbox[i]);
						prevBboxCnt += bucketsCount[i];

						costs[i] = 1.0f + (prevBbox.GetSurfaceArea() * prevBboxCnt
							+ suffixBbox[i + 1].GetSurfaceArea() * (totBbox - prevBboxCnt)) / aabb.GetSurfaceArea();
						if (costs[i] < costs[splitPos]) {
							splitPos = i;
						}
					}

					float leafCost = primsCnt;
					if (costs[splitPos] < leafCost) {
						int mid = std::partition(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
							[&](const Primitive& p) {
								int b = SAH_SPLIT_BUCKETS_NUM *
									centroid.Offset(p.GetCentroid())[axis];

								if (b == SAH_SPLIT_BUCKETS_NUM) {
									b -= 1;
								}

								return b <= splitPos;
							}) - m_Primitives.begin();

						int l = BuildTreeRecursion(indexL, mid);
						int r = BuildTreeRecursion(mid, indexR);

						m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, aabb, axis);
					}
					else {
						// create leaf directly, ignoring LEAF_PRIMS
						m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, aabb);
					}

					break;
				}
			}
		}

		return nodeIndex;
	}

}
