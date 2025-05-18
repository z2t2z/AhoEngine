#include "Ahopch.h"
#include "BVH.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Ray.h"
#include "Primitive.h"
#include "BBox.h"
#include "Runtime/Core/Math/Math.h"

namespace Aho {
	bool BVHi::Intersect(const Ray& ray) {
		float t = -1.0f;
		return IntersectNearest_loop(ray, t);
	}

	void BVHi::ApplyTransform(const glm::mat4& transform) {
		AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::BLAS);
		AHO_CORE_ASSERT(m_PrimitiveComp.size() == m_Primitives.size());

		for (size_t i = 0; i < m_Primitives.size(); i++) {
			AHO_CORE_ASSERT(m_Primitives[i].GetPrimId() >= 0 && m_Primitives[i].GetPrimId() <= m_PrimitiveComp.size());
			m_Primitives[i].ApplyTransform(transform, m_PrimitiveComp[m_Primitives[i].GetPrimId()]);
		}

		m_Nodes.clear();
		m_Root = BuildTreeRecursion(0, m_Primitives.size());
	}

	void BVHi::UpdateTLAS() {
		AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS);

		for (auto& blasPrim : m_Primitives) {
			const BVHi* blas = m_BLAS[blasPrim.GetPrimId()];
			blasPrim.SetBBox(blas->GetBBox());
		}

		m_Nodes.clear();
		m_Root = BuildTreeRecursion(0, m_Primitives.size());

		m_OffsetMap.clear();
		s_NodeOffset = 0;
		s_PrimOffset = 0;
		int offset = 0;
		for (PrimitiveDesc& blasPrim : m_Primitives) {
			blasPrim.SetID(offset++);
			const BVHi* blas = m_BLAS[blasPrim.GetPrimId()];

			m_OffsetMap.emplace_back(blas->GetMeshId(), s_NodeOffset, s_PrimOffset);
			s_NodeOffset += blas->GetNodeCount();
			s_PrimOffset += blas->GetPrimsCount();
		}
	}

	void BVHi::AddBLASPrimtive(const BVHi* blas) {
		AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS);
		m_Primitives.emplace_back(blas, m_BLAS.size());
		m_BLAS.push_back(const_cast<BVHi*>(blas));
	}

	bool BVHi::IntersectNearest_recursion(const Ray& ray, int root) {
		AHO_CORE_ASSERT(root >= 0 && root < m_Nodes.size());
		const BVHNodei& node = m_Nodes[root];
		if (!node.bbox.Intersect(ray)) {
			return false;
		}

		if (node.IsLeaf()) {
			for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
				AHO_CORE_ASSERT(i < m_Primitives.size());
				const PrimitiveDesc& p = m_Primitives[i];
				if (p.Intersect(ray)) {
					return true;
				}
			}
			return false;
		}
		return IntersectNearest_recursion(ray, node.left) ? true : IntersectNearest_recursion(ray, node.right);
	}

	bool BVHi::IntersectNearest_loop(const Ray& ray, float& candidate_t) {
		int n = m_Nodes.size();
		std::stack<int> stk;
		stk.push(m_Root);

		int find = -1;
		while (!stk.empty()) {
			int u = stk.top();
			stk.pop();

			const BVHNodei& node = m_Nodes[u];

			float t0 = -1.0f;
			if (!node.bbox.IntersectNearest(ray, t0)) {
				continue;
			}
			if (candidate_t > 0.0f && t0 > candidate_t) {
				continue;
			}

			if (node.IsLeaf()) {
				if (m_BvhLevel == BVHLevel::BLAS) {

					for (int i = node.firstPrimsIdx, cnt = 0; cnt < node.primsCnt; i++, cnt++) {
						AHO_CORE_ASSERT(i < m_Primitives.size());
						const PrimitiveDesc& p = m_Primitives[i];
						float t1 = -1.0f;
						if (p.IntersectNearest(ray, t1)) {
							if (candidate_t > 0.0f) {
								if (candidate_t > t1) {
									candidate_t = t1;
								}
							}
							else {
								candidate_t = t1;
							}
						}
					}
				
				}
				else {
					int id = m_Primitives[node.firstPrimsIdx].GetPrimId();
					AHO_CORE_ASSERT(id >= 0 && id <= m_BLAS.size());
					auto p = m_BLAS[id];
					//p->GetBBox().IntersectNearest(ray, candidate_t);
					p->IntersectNearest_loop(ray, candidate_t);
				}
			}
			else {
				stk.push(node.right);
				stk.push(node.left);
			}
		}

		return candidate_t > 0.0f;
	}

	void BVHi::Build(const std::shared_ptr<StaticMesh>& mesh) {
		AHO_CORE_ASSERT(false);
		//m_Primitives.reserve(mesh->GetVerticesCount() / 3);
		//const auto& meshInfo = mesh->GetMeshInfo();
		//for (const auto& info : meshInfo) {
		//	const auto& vertices = info->vertexBuffer;
		//	const auto& indices = info->indexBuffer;
		//	size_t siz = indices.size();
		//	AHO_CORE_ASSERT(siz % 3 == 0);
		//	for (size_t i = 0; i < siz; i += 3) {
		//		const Vertex& v0 = vertices[indices[i]];
		//		const Vertex& v1 = vertices[indices[i + 1]];
		//		const Vertex& v2 = vertices[indices[i + 2]];
		//		m_Primitives.emplace_back(v0, v1, v2, i / 3);
		//		m_PrimitiveComp.emplace_back(v0, v1, v2);
		//	} 
		//}

		//m_Nodes.reserve(m_Primitives.size());
		//m_Root = BuildTreeRecursion(0, m_Primitives.size());
		//m_Nodes.shrink_to_fit();
	}
	
	void BVHi::Build(const std::shared_ptr<MeshInfo>& info) {
		const auto& vertices = info->vertexBuffer;
		const auto& indices = info->indexBuffer;
		size_t siz = indices.size();
		AHO_CORE_ASSERT(siz % 3 == 0);

		m_Primitives.reserve(siz);
		for (size_t i = 0; i < siz; i += 3) {
			const Vertex& v0 = vertices[indices[i]];
			const Vertex& v1 = vertices[indices[i + 1]];
			const Vertex& v2 = vertices[indices[i + 2]];
			m_Primitives.emplace_back(v0, v1, v2, i / 3, m_MeshId);
			m_PrimitiveComp.emplace_back(v0, v1, v2);
		}

		m_Nodes.reserve(m_Primitives.size());
		m_Root = BuildTreeRecursion(0, m_Primitives.size());
		m_Nodes.shrink_to_fit();

		AHO_CORE_ASSERT(m_Root == 0);
		m_Nodes[m_Root].meshId = m_MeshId;
	}

	int BVHi::BuildTreeRecursion(int indexL, int indexR) {
		AHO_CORE_ASSERT(indexL < indexR);
		int nodeIndex = m_Nodes.size();

		m_Nodes.push_back(BVHNodei());
		m_Nodes[nodeIndex].meshId = m_MeshId;

		BBox bbox, centroid;
		for (int i = indexL; i < indexR; i++) {
			const auto& p = m_Primitives[i];
			bbox.Merge(p.GetBBox());
			centroid.Merge(p.GetCentroid());
		}

		int primsCnt = indexR - indexL;
		if (primsCnt <= MAX_LEAF_PRIMS) {
			m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, bbox);
		}
		else {
			int axis = (int)bbox.GetSplitAxis();

			switch (m_SplitMethod) {
			case (SplitMethod::NAIVE): {

				//AHO_CORE_ASSERT(m_BvhLevel == BVHLevel::TLAS);
				std::sort(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
					[axis](const PrimitiveDesc& lhs, const PrimitiveDesc& rhs) {
						return lhs.GetCentroid()[axis] < rhs.GetCentroid()[axis];
					});

				int l = BuildTreeRecursion(indexL, indexL + primsCnt / 2);
				int r = BuildTreeRecursion(indexL + primsCnt / 2, indexR);

				m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, bbox, axis);
				break;
			}

			case (SplitMethod::SAH): {

				std::array<int, SAH_SPLIT_BUCKETS_NUM> bucketsCount{};
				std::array<BBox, SAH_SPLIT_BUCKETS_NUM> bucketsBBox{};

				for (int i = indexL; i < indexR; i++) {
					const auto& p = m_Primitives[i];
					int b = SAH_SPLIT_BUCKETS_NUM *
						centroid.Offset(p.GetCentroid())[axis];

					if (b == SAH_SPLIT_BUCKETS_NUM) {
						b -= 1;
					}

					bucketsCount[b] += 1;
					bucketsBBox[b].Merge(p.GetBBox());
				}

				std::array<BBox, 1 + SAH_SPLIT_BUCKETS_NUM> suffixBBox{};
				for (int i = SAH_SPLIT_BUCKETS_NUM - 1; i > 0; i--) {
					suffixBBox[i].Merge(suffixBBox[i + 1]);
					suffixBBox[i].Merge(bucketsBBox[i]);
				}

				BBox prevBBox;
				int totBBox = primsCnt;
				int prevBBoxCnt = 0;
				std::array<double, SAH_SPLIT_BUCKETS_NUM> costs{};
				std::fill(costs.begin(), costs.end(), inf);
				int splitPos = -1;
				// split into two sets: [0, i], [i + 1, SAH_SPLIT_BUCKETS_NUM - 1]
				for (int i = 0; i < SAH_SPLIT_BUCKETS_NUM - 1; i++) {
					prevBBox.Merge(bucketsBBox[i]);
					prevBBoxCnt += bucketsCount[i];
					if (bucketsCount[i] == 0) {
						continue;
					}
					if (prevBBoxCnt == totBBox) {
						break;
					}

					costs[i] = 1.0f + (prevBBox.GetSurfaceArea() * prevBBoxCnt + suffixBBox[i + 1].GetSurfaceArea() * (totBBox - prevBBoxCnt)) / bbox.GetSurfaceArea();

					if (splitPos == -1 || costs[i] < costs[splitPos]) {
						splitPos = i;
					}
				}

				float leafCost = primsCnt;
				if (splitPos != -1 && costs.at(splitPos) < leafCost) {
					int mid = std::partition(m_Primitives.begin() + indexL, m_Primitives.begin() + indexR,
						[&](const PrimitiveDesc& p) {
							int b = SAH_SPLIT_BUCKETS_NUM *
								centroid.Offset(p.GetCentroid())[axis];

							if (b == SAH_SPLIT_BUCKETS_NUM) {
								b -= 1;
							}

							return b <= splitPos;
						}) - m_Primitives.begin();

					int l = BuildTreeRecursion(indexL, mid);
					int r = BuildTreeRecursion(mid, indexR);

					m_Nodes[nodeIndex].InitInterior(nodeIndex, l, r, bbox, axis);
				}
				else {
					// create leaf directly, ignoring MAX_LEAF_PRIMS
						m_Nodes[nodeIndex].InitLeaf(nodeIndex, indexL, primsCnt, bbox);
					}

					break;
				}
			}
		}

		return nodeIndex;
	}

}

