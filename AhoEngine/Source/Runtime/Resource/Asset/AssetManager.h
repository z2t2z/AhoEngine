#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/AssetCreator/AssetCreator.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>

/* 
	Referred to Piccolo, basically just for editor.
	Also need a runtimeAssetManager in the far future, 
	which is likely to have entirely different behaviors.
*/

/*
	Definition of the base asset class in ResourceType.h 
	We should not define asset realted to renderables, scenes, etc. in Resource layer.
	Assets should be mantained as a DAG(or not? Since we need to load/unload in different levels in runtime).
	In Function layer we will define renderables which consists of some base assets here,like StaticMesh and Image.
	Basically in this layer we transform DCC files to our own asset files that can be easily used later.
*/

/*
	AssetManager is responsible for:
	1. Loading DCC file and convert it into our asset file (DCC -> JSON)
	2. Loading resources from our assfile(JSON)
*/

namespace Aho {
	class AssetManager {
	public:
		AssetManager() = default;
		~AssetManager() = default;

		template<typename AssetType>
		bool LoadAssetFromFile(const std::filesystem::path& path, AssetType& assetOut) {
			if (path.extension().string() != ".asset") {
				return CreateAsset(path, assetOut);
			}
			std::ifstream rawJson(path.string());
			if (!rawJson) {
				AHO_CORE_ERROR("Failed to open file: {}", path.string());
				return false;
			}
			const nlohmann::json& parsedJson = nlohmann::json::parse(rawJson);
			Serializer::Deserialize(parsedJson, assetOut);
			return true;
		}

		template<typename AssetType>
		bool SaveAsset(const std::filesystem::path& path, AssetType& assetIn) {
			/* TODO */
		}

	private:
		template<typename AssetType>
		bool CreateAsset(const std::filesystem::path& path, AssetType& assetOut) {
			const auto& fileExt = path.extension().string();
			if (fileExt == ".obj" || fileExt == ".fbx") {
				assetOut = *(AssetCreater::MeshAssetCreater(path.string()));
				return true;
			}
			AHO_CORE_ASSERT("Not supported yet");
			return false;
		}
		void Initialize() { /* TODO */ }

	private:
		std::unordered_map<UUID, std::shared_ptr<Asset>> s_AssetPools;
	};

}