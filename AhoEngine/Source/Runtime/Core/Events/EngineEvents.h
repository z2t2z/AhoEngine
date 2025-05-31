#pragma once

#include "_Event.h"
#include <string>
#include <stdint.h>
#include <memory>

namespace Aho {
	class ShaderAsset;
	enum class ShaderUsage;
	enum class ShaderFeature : uint32_t;
	namespace EngineEvents {
		inline _Event<const std::string, const std::shared_ptr<ShaderAsset>> OnShaderAssetReload;
		inline _Event<ShaderUsage, ShaderFeature, ShaderFeature> OnShaderFeatureChanged;
	}
}
