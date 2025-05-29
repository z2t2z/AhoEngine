#pragma once

#include "_Event.h"
#include "EventBus.h"
#include <string>

namespace Aho {
	class ShaderAsset;
	namespace EngineEvents {
		inline Event<const std::string, ShaderAsset*> OnShaderAssetReload;
	}
}
