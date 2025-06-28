#include "Ahopch.h"
#include "Components.h"

namespace Aho {
	int PointLightComponent::s_PointLightCnt = 0;
	int AnimatorComponent::s_BoneOffset = 0;

	std::unique_ptr<_Texture> IBLComponent::BRDFLUT = nullptr;
}