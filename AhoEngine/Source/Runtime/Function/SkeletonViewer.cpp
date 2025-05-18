#include "Ahopch.h"
#include "SkeletonViewer.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"

namespace Aho {
	void SkeletonViewer::Update(BoneNode* curr, const glm::mat4& updateMatrix) {
		if (m_NodeIndexMap.contains(curr)) {
			m_BoneTransform[m_NodeIndexMap.at(curr)] = updateMatrix;
			m_TransformMap[curr] = updateMatrix;
		}
		else {
			AHO_CORE_ERROR("huh");
		}
	}

	void SkeletonViewer::BuildSkeleton() {
		auto dfs = 
			[this](auto self, BoneNode* curr, const BoneNode* parent, glm::vec3 parentPos, glm::mat4 globalMatrix) -> void {
				//AHO_CORE_ASSERT(curr->hasInfluence);
				glm::mat4 localMatrix = curr->transform;
				globalMatrix = globalMatrix * localMatrix;
				const BoneNode* nxtParent = parent;
				glm::vec3 nxtParentPoint = parentPos;
				if (curr->hasInfluence || curr->children.empty()) {
					glm::vec3 currPos = glm::vec3(globalMatrix[3]);
					if (parent && parentPos != glm::vec3(0.0f)) {		// Assume the root node is fixed at the origin
						glm::vec3 PtoC = currPos - parentPos;
						glm::vec3 d(0.0f, -1.0f, 0.0f);
						glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::length(PtoC)));
						auto axis = glm::cross(d, PtoC);
						float angle = glm::acos(glm::dot(d, glm::normalize(PtoC)));
						glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
						glm::mat4 rotate = glm::toMat4(q);
						glm::mat4 translation = glm::translate(glm::mat4(1.0f), parentPos);
						m_NodeIndexMap[curr] = m_BoneTransform.size();
						m_BoneTransform.push_back(translation * rotate * scale);
						m_TransformMap[curr] = translation * rotate * scale;
					}
					nxtParent = curr;
					nxtParentPoint = currPos;
				}
				for (const auto& child : curr->children) {
					self(self, child, nxtParent, nxtParentPoint, globalMatrix);
				}
			};
		dfs(dfs, m_Root, nullptr, glm::vec3(0.0f), glm::mat4(1.0f)); // correct, ignore extra bones come in with assimp
	}
}