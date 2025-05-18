#pragma once

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <map>
#include <set>

namespace Aho {
	struct BoneNode;

	// Helper for visualizing the skeleton
	class SkeletonViewer {
	public:
		SkeletonViewer(BoneNode* root) : m_Root(root) {
			BuildSkeleton();
		}
		~SkeletonViewer() = default;

		std::vector<glm::mat4>& GetBoneTransform() { return m_BoneTransform; }
		std::map<BoneNode*, uint32_t>& GetBoneNodeIndexMap() { return m_NodeIndexMap; }
		std::map<BoneNode*, glm::mat4>& GetTransformMap() { return m_TransformMap; }

		void Update(BoneNode* curr, const glm::mat4& updateMatrix);
		// Update visual skeleton or not
		void SetShouldUpdate(bool state) { m_ShouldUpd = state; }
		// Update visual skeleton or not
		bool ShouldUpdate() { return m_ShouldUpd; }
	private:
		void BuildSkeleton();
	private:
		bool m_ShouldUpd{ true };
		std::map<BoneNode*, uint32_t> m_NodeIndexMap;
		std::map<BoneNode*, glm::mat4> m_TransformMap;
		std::vector<glm::mat4> m_BoneTransform;
		BoneNode* m_Root;
	};

} // namespace Aho
