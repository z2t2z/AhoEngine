#pragma once

#include "BBox.h"

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
	constexpr int MAX_LEAF_PRIMS = 1;

	struct alignas(16) BVHNodei {
		BVHNodei() : left(-1), right(-1), nodeIdx(-1), firstPrimsIdx(-1), primsCnt(-1), axis(-1) {}
		void InitLeaf(int nodeIdx_, int firstPrimsIdx_, int primsCnt_, const BBox& bbox_) {
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
}