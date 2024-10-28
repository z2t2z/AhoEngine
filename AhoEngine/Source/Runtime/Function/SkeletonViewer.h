#pragma once
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include "Runtime/Function/Renderer/VertexArrayr.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <map>
#include <set>

namespace Aho {
	// Helper for visualizing the skeleton
	class SkeletonViewer {
	public:
		SkeletonViewer(BoneNode* root) : m_Root(root) {
			BuildSkeleton();
		}
		~SkeletonViewer() = default;
		const std::vector<glm::mat4>& GetBoneTransform() { return m_BoneTransform; }
		const std::map<const BoneNode*, uint32_t>& GetBoneNodeIndexMap() { return m_NodeIndexMap; }
		void Update(const BoneNode* curr, const glm::mat4& updateMatrix) {
			if (m_NodeIndexMap.contains(curr)) {
				m_BoneTransform[m_NodeIndexMap.at(curr)] = updateMatrix;
			}
			else {
				AHO_CORE_ERROR("huh");
			}
		}
		void SetShouldUpdate(bool state) { m_ShouldUpd = state; }
		bool ShouldUpdate() { return m_ShouldUpd; }

	private:
		void BuildSkeleton() {
			auto dfs = [&](auto self, const BoneNode* curr, const BoneNode* parent, glm::vec3 parentPos, glm::mat4 globalMatrix) -> void {
				glm::mat4 localMatrix = curr->transform;
				globalMatrix = globalMatrix * localMatrix;
				glm::vec3 currPos = glm::vec3(globalMatrix[3]);
				const BoneNode* nxtParent = parent;
				glm::vec3 nxtParentPoint = parentPos;
				if (curr->bone.name.find("$AssimpFbx$") == std::string::npos) {
					if (parent && parentPos != glm::vec3(0.0f)) {		// Assume the root node is fixed at the origin
						glm::vec3 CtoP = parentPos - currPos;
						glm::vec3 up(0.0f, 1.0f, 0.0f);
						glm::mat4 scale = glm::scale(glm::mat4(1.0f), -glm::vec3(glm::length(CtoP)));
						auto axis = glm::cross(up, CtoP);
						float angle = glm::acos(glm::dot(glm::normalize(up), glm::normalize(CtoP)));
						glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
						glm::mat4 rotate = glm::toMat4(q);
						glm::mat4 translation = glm::translate(glm::mat4(1.0f), parentPos);
						m_NodeIndexMap[curr] = m_BoneTransform.size();
						m_BoneTransform.push_back(translation * rotate * scale);
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
	private:
		bool m_ShouldUpd{ true };
		std::map<const BoneNode*, uint32_t> m_NodeIndexMap;
		std::vector<glm::mat4> m_BoneTransform;
		BoneNode* m_Root;
	};

	static void CreateUnitSphere(float x, float y, float z, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
		const float t = (1.0 + sqrt(5.0)) / 2.0;
		std::vector<glm::vec3> baseVertices = {
			{-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
			{ 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
			{ t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
		};
		for (auto& v : baseVertices) {
			v = glm::normalize(v) + glm::vec3(x, y, z);
			vertices.push_back(v.x);
			vertices.push_back(v.y);
			vertices.push_back(v.z);
		}
		std::vector<unsigned int> baseIndices = {
			 0, 11,  5,    0,  5,  1,    0,  1,  7,    0,  7, 10,    0, 10, 11,
			 1,  5,  9,    5, 11,  4,   11, 10,  2,   10,  7,  6,    7,  1,  8,
			 3,  9,  4,    3,  4,  2,    3,  2,  6,    3,  6,  8,    3,  8,  9,
			 4,  9,  5,    2,  4, 11,    6,  2, 10,    8,  6,  7,    9,  8,  1
		};
		for (auto& idx : baseIndices) {
			idx += indices.size();
		}
		indices.insert(indices.end(), baseIndices.begin(), baseIndices.end());
	}

} // namespace Aho
