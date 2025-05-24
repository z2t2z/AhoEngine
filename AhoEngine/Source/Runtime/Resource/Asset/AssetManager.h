#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "AssetLoadOptions.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"
#include "Runtime/Resource/Asset/Mesh/MeshAsset.h"
#include "Runtime/Resource/Asset/AssetLoaders.h"
#include "Runtime/Resource/FileWatcher/FileWatcher.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/Shader/ShaderVariantManager.h"

#include <fstream>
#include <filesystem>
#include <mutex>
#include <thread>

namespace Aho {
	class Mesh;
	class AssetManager {
	public:
		AssetManager() = default;
		~AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		AssetManager(AssetManager&&) = delete;
		AssetManager& operator=(AssetManager&&) = delete;

		template<typename AssetT, typename LoadOptionsT> 
		std::shared_ptr<AssetT> _LoadAsset(const std::shared_ptr<EntityManager>& emg, const LoadOptionsT& opts) {
			static_assert(std::is_base_of<Asset, AssetT>::value, "AssetT must derive from Asset");

			if constexpr (std::is_same_v<AssetT, TextureAsset>) {
				if (m_Assets.count(opts.path)) {
					return std::static_pointer_cast<TextureAsset>(m_Assets.at(opts.path));
				}

				std::shared_ptr<TextureAsset> texAsset = std::make_shared<TextureAsset>(opts.path);
				emg->AddComponent<TextureAssetComponent>(emg->CreateEntity(), texAsset);
				m_Assets[opts.path] = texAsset;
				return texAsset;
			}

			if constexpr (std::is_same_v<AssetT, MaterialAsset>) {
				if (m_Assets.count(opts.path)) {
					return std::static_pointer_cast<MaterialAsset>(m_Assets.at(opts.path));
				}

				std::shared_ptr<MaterialAsset> matAsset = std::make_shared<MaterialAsset>(opts.path);
				emg->AddComponent<MaterialAssetComponent>(emg->CreateEntity(), matAsset);
				m_Assets[opts.path] = matAsset;
				return matAsset;
			}

			if constexpr (std::is_same_v<AssetT, AnimationAsset>) {
				AHO_CORE_ASSERT(false, "Not yet implemented");
			}

			if constexpr (std::is_same_v<AssetT, MeshAsset>) {
				return _LoadMeshAsset(emg, opts);
			}

			if constexpr (std::is_same_v<AssetT, ShaderAsset>) {
				return _LoadShaderAsset(g_RuntimeGlobalCtx.m_EntityManager, opts);
			}

			return nullptr;
		}

		std::shared_ptr<Asset> LoadAsset(const std::filesystem::path& path) {
			if (m_Assets.count(path.string())) {
				return m_Assets.at(path.string());
			}
			auto ext = path.extension().string();
			auto bext = ext;
			for (char& c : ext) {
				c = tolower(c);
			}
			static std::unordered_set<std::string> s_TextureExt = { ".dds", ".hdr", ".exr", ".png", ".jpg", ".jpeg" };
			if (s_TextureExt.contains(ext)) {
				std::shared_ptr<TextureAsset> texAsset = std::make_shared<TextureAsset>(path.string());
				m_Assets[path.string()] = texAsset;
				return texAsset;
			}

			static std::unordered_set<std::string> s_MeshExt = { ".obj", ".fbx" };
			if (s_MeshExt.contains(ext)) {
				//std::shared_ptr<MeshAsset> meshAsset = 
			}

			AHO_CORE_ASSERT(false);
			return nullptr;
		}

		template<typename AssetType>
		bool LoadAssetFromFile(const std::filesystem::path& path, AssetType& assetOut, const glm::mat4& preTransform = glm::mat4(1.0f)) {
			return false;
		}

	private:
		std::shared_ptr<MeshAsset> _LoadMeshAsset(const std::shared_ptr<EntityManager>& emg, const MeshOptions& opts);
		std::shared_ptr<ShaderAsset> _LoadShaderAsset(const std::shared_ptr<EntityManager>& emg, const ShaderOptions& opts);

	private:
		template<typename AssetType>
		bool CreateAsset(const std::filesystem::path& path, AssetType& assetOut, const glm::mat4& preTransform) {
			return false;
		}

		void Initialize() { /* TODO */ }

		void LoadMeshAsset(const MeshOptions& loadOptions, EntityManager& emg) {
			//// 1. Parse mesh file into raw mesh and material data
			//std::vector<Mesh>     rawMeshes;
			//std::vector<MaterialPaths> matPaths;
			//bool success = AssetLoader::MeshLoader(loadOptions, rawMeshes, matPaths);
			//if (!success) {
			//	AHO_ASSERT(false);
			//	return;
			//}


			//// 2. Create MaterialAsset entities first
			//std::vector<entt::entity> materialEntities;
			//materialEntities.reserve(matPaths.size());
			//for (size_t i = 0; i < matPaths.size(); ++i) {
			//	const auto& mat = matPaths[i];
			//	auto matEntity = ecs.create();
			//	ecs.emplace<MaterialAssetComponent>(matEntity, std::make_shared<MaterialAsset>(mat));
			//	materialEntities.push_back(matEntity);
			//}

			//// 3. Create MeshAsset entities and link to MaterialAsset entities
			//for (size_t i = 0; i < rawMeshes.size(); ++i) {
			//	const auto& mesh = rawMeshes[i];
			//	auto meshEntity = ecs.create();
			//	// attach raw mesh data
			//	ecs.emplace<MeshAssetComponent>(meshEntity, std::make_shared<MeshAsset>(mesh));
			//	// link corresponding material
			//	if (i < materialEntities.size()) {
			//		ecs.emplace<MaterialRefComponent>(meshEntity, materialEntities[i]);
			//	}
			//}
		}

	private:
		FileWatcher m_Filewatcher;
		std::unordered_map<std::string, std::shared_ptr<Asset>> m_Assets;
	};

}