#pragma once
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"

namespace Aho {
	class Animator {
	public:
		static void Update(float currTime, std::vector<glm::mat4>& globalMatrices, const BoneNode* root, const std::shared_ptr<AnimationAsset>& anim) {
			glm::mat4 globalTrans = glm::mat4(1.0f);
			UpdateBoneTree(currTime, globalMatrices, root, anim, globalTrans);
		}
	private:
		static void UpdateBoneTree(float currTime, std::vector<glm::mat4>& globalMatrices,
			const BoneNode* currNode, const std::shared_ptr<AnimationAsset>& anim, glm::mat4& globalTrans) {
			int id = currNode->bone.id;
			std::string name = currNode->bone.name;
			const auto& positions = anim->GetPositions(id);
			const auto& rotations = anim->GetRotations(id);
			const auto& scales = anim->GetScales(id);
			glm::mat4 localTransform = glm::mat4(1.0f);
			glm::mat4 scale = glm::mat4(1.0f);
			glm::mat4 trans = glm::mat4(1.0f);
			glm::mat4 rot = glm::mat4(1.0f);
			if (currNode->bone.hasAnim) {
				if (!scales.empty()) {
					if (scales.size() == 1) {
						scale = glm::scale(glm::mat4(1.0f), scales[0].attribute);
					}
					else {
						int prev = GetKeyframeIndex(currTime, scales);
						int nxt = prev + 1;
						scale = Lerp(currTime, scales[prev], scales[nxt]);
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
			}
			auto finalTrans = trans * rot * scale;
			//auto finalTrans = scale * rot * trans;
			globalTrans = globalTrans * finalTrans;
			if (currNode->bone.hasAnim) {
				globalMatrices[id] = globalTrans * currNode->bone.offset;
			}
			for (auto& childNode : currNode->children) {
				UpdateBoneTree(currTime, globalMatrices, childNode, anim, globalTrans);
			}
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