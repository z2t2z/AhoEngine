#pragma once
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"

namespace Aho {
	static bool s_SkeletalVisualized = true;
	static bool g_PrintOnce = true;
	static bool g_BindingPose = false;
	class Animator {
	public:
		static void Update(float currTime, std::vector<glm::mat4>& globalMatrices,
							BoneNode* root, const std::shared_ptr<AnimationAsset>& anim, SkeletonViewer* viewer) {
			g_PrintOnce = false;
			UpdateSkeletonTree(currTime, globalMatrices, glm::mat4(1.0f), anim, root, nullptr, glm::vec3(0.0f), viewer);
		}

		static void ApplyPose(std::vector<glm::mat4>& globalMatrices, BoneNode* root) {
			auto dfs = [&](auto self, BoneNode* curr) -> void {
				if (curr->hasInfluence) {
					globalMatrices[curr->bone.id] = globalMatrices[curr->bone.id] * curr->bone.offset;
				}
				for (auto child : curr->children) {
					self(self, child);
				}
			};
			dfs(dfs, root);
		}

	private:
		static void UpdateSkeletonTree(float currTime, std::vector<glm::mat4>& globalMatrices, glm::mat4 globalTrans, const std::shared_ptr<AnimationAsset>& anim,
										BoneNode* currNode, BoneNode* validParent, glm::vec3 parentPos, SkeletonViewer* viewer) {
			glm::mat4 localTransform = (g_BindingPose ? currNode->transform : GetAnimationFinalMatrix(currTime, currNode, anim)) * currNode->transformParam->GetTransform();
			globalTrans = globalTrans * localTransform;
			currNode->global = globalTrans;
			BoneNode* nxtValidParent = validParent;
			glm::vec3 nxtParentPos = parentPos;
			if (g_PrintOnce) {
				AHO_CORE_WARN("P: {}, Cur: {}", (validParent ? validParent->bone.name : "null"), currNode->bone.name);
			}
			if (viewer->ShouldUpdate()) {
				if (currNode->hasInfluence || currNode->children.empty()) {
					glm::vec3 currPos(globalTrans[3]);
					if (validParent && parentPos != glm::vec3(0.0f)) {
						glm::vec3 PtoC = currPos - parentPos;
						glm::vec3 d(0.0f, -1.0f, 0.0f);
						glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::length(PtoC)));
						auto axis = glm::cross(d, PtoC);
						float angle = glm::acos(glm::dot(d, glm::normalize(PtoC)));
						glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
						glm::mat4 rotation = glm::toMat4(q);
						glm::mat4 translation = glm::translate(glm::mat4(1.0f), parentPos);
						viewer->Update(currNode, translation * rotation * scale);
					}
					nxtValidParent = currNode;
					nxtParentPos = currPos;
				}
				else {
					//AHO_CORE_ASSERT(false);
				}
			}
			if (currNode->hasInfluence) {
				globalMatrices[currNode->bone.id] = globalTrans; // *currNode->bone.offset;
			}
			currNode->global = globalTrans;
			for (auto child : currNode->children) {
				UpdateSkeletonTree(currTime, globalMatrices, globalTrans, anim, child, nxtValidParent, nxtParentPos, viewer);
			}
		}

		static glm::mat4 GetAnimationFinalMatrix(float currTime, const BoneNode* currNode, const std::shared_ptr<AnimationAsset>& anim) {
			if (!currNode->hasInfluence) {
				return glm::mat4(1.0f);
			}
			int id = currNode->bone.id;
			std::string name = currNode->bone.name;
			const auto& positions = anim->GetPositions(id);
			const auto& rotations = anim->GetRotations(id);
			const auto& scales = anim->GetScales(id);
			glm::mat4 scale = glm::mat4(1.0f);
			glm::mat4 trans = glm::mat4(1.0f);
			glm::mat4 rot = glm::mat4(1.0f);
			if (!scales.empty()) {
				if (scales.size() == 1) {
					scale = glm::scale(glm::mat4(1.0f), scales[0].attribute);
				}
				else {
					int prev = GetKeyframeIndex(currTime, scales);
					int nxt = prev + 1;
					glm::vec3 finalScale = glm::mix(scales[prev].attribute, scales[nxt].attribute,
						GetInterpolationFactor(scales[prev].timeStamp, scales[nxt].timeStamp, currTime));
					scale = glm::scale(glm::mat4(1.0f), finalScale);
				}
			}
			if (!rotations.empty()) {
				if (rotations.size() == 1) {
					rot = glm::toMat4(glm::normalize(rotations[0].attribute));
				}
				else {
					int prev = GetKeyframeIndex(currTime, rotations);
					int nxt = prev + 1;
					rot = Slerp(currTime, rotations[prev], rotations[nxt]);
				}
			}
			if (!positions.empty()) {
				if (positions.size() == 1) {
					trans = glm::translate(glm::mat4(1.0f), positions[0].attribute);
				}
				else {
					int prev = GetKeyframeIndex(currTime, positions);
					int nxt = prev + 1;
					trans = Lerp(currTime, positions[prev], positions[nxt]);
				}
			}
			return trans * rot * scale;
		}

		template<typename T>
		static int GetKeyframeIndex(float currTime, const T& keyframeVec) {
			auto it = std::lower_bound(keyframeVec.begin(), keyframeVec.end(), currTime, [](const auto& info, float currTime) {
				return info.timeStamp < currTime;
				});
			return std::max(0, int(std::prev(it) - keyframeVec.begin()));
		}

		static float GetInterpolationFactor(float prevTime, float nxtTime, float currTime) {
			return (currTime - prevTime) / (nxtTime - prevTime);
		}

		template<typename T>
		static glm::mat4 Lerp(float currTime, const T& lhs, const T& rhs) {
			float scaleFactor = GetInterpolationFactor(lhs.timeStamp, rhs.timeStamp, currTime);
			glm::vec3 res = glm::mix(lhs.attribute, rhs.attribute, scaleFactor);
			return glm::translate(glm::mat4(1.0f), res);
		}

		template<typename T>
		static glm::mat4 Slerp(float currTime, const T& lhs, const T& rhs) {
			float scaleFactor = GetInterpolationFactor(lhs.timeStamp, rhs.timeStamp, currTime);
			glm::quat res = glm::slerp(lhs.attribute, rhs.attribute, scaleFactor);
			res = glm::normalize(res);
			return glm::toMat4(res);
		}
	};
}