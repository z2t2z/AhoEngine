#pragma once
#include "Runtime/Resource/Asset/Asset.h"
#include "Bone.h"
#include <unordered_map>

namespace Aho {
	class SkeletalAsset : public Asset {
	public:
		SkeletalAsset(BoneNode* root) : m_Root(root) {}
		~SkeletalAsset() { delete m_Root; }
		BoneNode* GetRoot() { return m_Root; }
		void SetRoot(BoneNode* root) { m_Root = root; }
	private:
		BoneNode* m_Root;
	};


	template<typename T>
	using KeyframeArray2D = std::vector<std::vector<T>>;
	class AnimationAsset : public Asset {
	public:
		AnimationAsset(const KeyframeArray2D<KeyframePosition>& pos, 
						const KeyframeArray2D<KeyframeRotation>& rot, 
						const KeyframeArray2D<KeyframeScale>& scale, 
						float duration, float ticksPerSecond,
						const glm::mat4& inv) 
			: m_Positions(pos), m_Rotations(rot), m_Scales(scale), m_Duration(duration), m_TicksPerSecond(ticksPerSecond), m_CurrTime(0.0f), m_GlobalInverse(inv) {}
		~AnimationAsset() = default;
		inline int GetTicksPerSecond() { return m_TicksPerSecond; }
		inline float GetDuration() { return m_Duration; }
		const std::vector<KeyframePosition>& GetPositions(int idx) { return m_Positions[idx]; }
		const std::vector<KeyframeRotation>& GetRotations(int idx) { return m_Rotations[idx]; }
		const std::vector<KeyframeScale>& GetScales(int idx) { return m_Scales[idx]; }
		size_t GetBoneCnt() { return m_Positions.size(); }
		const glm::mat4 GetGlobalInverse() { return m_GlobalInverse; }
	private:
		KeyframeArray2D<KeyframePosition> m_Positions;
		KeyframeArray2D<KeyframeRotation> m_Rotations;
		KeyframeArray2D<KeyframeScale> m_Scales;
	private:
		glm::mat4 m_GlobalInverse;
		float m_Duration;
		float m_CurrTime;
		int m_TicksPerSecond;
	};
}