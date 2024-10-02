#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "json.hpp"
#include <filesystem>

/* 
	Referred to Piccolo, basically just for editor.
	Also need a runtimeAssetManager in the far future, 
	which is likely to have entirely different behaviors.
*/

namespace Aho {
	class AssetManager {
	public:
		template<typename, AssetType>
		bool LoadAssetFromFile(const std::string& path, AssetType& assetOut) {
			std::ifstream rawJson(path);
			if (!rawJson) {
				AHO_CORE_ERROR("Failed to open file: {}", path);
				return false;
			}
			
			nlohmann::json& parsedJson = nlohmann::json::parse(rawJson);
			Serializer::Deserialize(parsedJson, assetOut);
			return true;
		}
	};

}