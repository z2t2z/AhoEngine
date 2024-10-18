#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/AssetCreator/AssetCreator.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <mutex>
#include <thread>

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

		// TODO: Maybe try std::shared_ptr<T> assetOut
		template<typename AssetType>
		bool LoadAssetFromFile(const std::filesystem::path& path, AssetType& assetOut) {
			if (m_AssetPaths.contains(path.string())) {
				/* TODO : pop out a window here */
				auto uuid = m_AssetPaths.at(path.string());
				assetOut = *(s_AssetPools.at(uuid)); // why?
				return true;
			}
			if (path.extension().string() != ".asset") {
				if (!CreateAsset(path, assetOut)) {
					AHO_CORE_ERROR("Import failed at path: {}", path.string());
				}
				return true;
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

		template<typename T>
		void AddAsset(const std::string& path, UUID uuid, const T& res) {
			s_AssetPools[uuid] = res;
			m_AssetPaths[path] = uuid;
		}
	private:
		template<typename AssetType>
		bool CreateAsset(const std::filesystem::path& path, AssetType& assetOut) {
			const auto& fileExt = path.extension().string();
			if (fileExt == ".obj" || fileExt == ".fbx" || fileExt == ".FBX" || fileExt == ".OBJ") {
				if (fileExt == ".fbx" || fileExt == ".FBX") {
					AHO_CORE_WARN(".fbx does not use a verbose vertex format which may leads to incorrect tangent vector calculation");
				}
				assetOut = *AssetCreator::MeshAssetCreater(path.string());
				return true;
			}
			AHO_CORE_ASSERT("Not supported yet");
			return false;
		}

		void Initialize() { /* TODO */ }

	private:
		std::unordered_map<UUID, std::shared_ptr<StaticMesh>> s_AssetPools;
		std::unordered_map<std::string, UUID> m_AssetPaths;
	};

}