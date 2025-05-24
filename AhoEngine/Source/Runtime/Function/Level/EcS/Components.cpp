#include "Ahopch.h"
#include "Components.h"

namespace Aho {
	int PointLightComponent::s_PointLightCnt = 0;
	int AnimatorComponent::s_BoneOffset = 0;

	std::vector<int> _BVHComponent::s_FreeIds;
	int _BVHComponent::s_Id = 0;

	std::unique_ptr<_Texture> IBLComponent::BRDFLUT = nullptr;
}