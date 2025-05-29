#include "Ahopch.h"
#include "AssetManager.h"

#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Core/Events/MainThreadDispatcher.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/Timer.h"

namespace Aho {
	std::shared_ptr<MeshAsset> AssetManager::_LoadMeshAsset(const std::shared_ptr<EntityManager>& emg, const MeshOptions& opts) {
		if (m_Assets.count(opts.path)) {
			return std::static_pointer_cast<MeshAsset>(m_Assets.at(opts.path));
		}

		std::vector<Mesh> meshes;
		std::vector<MaterialPaths> mats;
		bool success = AssetLoader::MeshLoader(opts, meshes, mats);
		AHO_CORE_ASSERT(success && meshes.size() == mats.size());

		std::vector<Entity> matEnts;
		matEnts.reserve(mats.size());
		for (const MaterialPaths& mat : mats) {
			std::shared_ptr<MaterialAsset> matAsset = std::make_shared<MaterialAsset>(opts.path, mat.UsagePaths);
			matEnts.push_back(emg->CreateEntity());
			emg->AddComponent<MaterialAssetComponent>(matEnts.back(), matAsset);
		}

		if (opts.BuildBVH) {
			if (emg->GetView<SceneBVHComponent>().empty()) {
				auto scene = emg->CreateEntity();
				emg->AddComponent<SceneBVHComponent>(scene);
			}
		}

		std::shared_ptr<MeshAsset> firstMeshAsset;
		for (size_t i = 0; i < meshes.size(); i++) {
			std::shared_ptr<MeshAsset> meshAsset = std::make_shared<MeshAsset>(opts.path, meshes[i].name, meshes[i]);
			if (i == 0) {
				firstMeshAsset = meshAsset;
			}
			Entity meshAssetEntity = emg->CreateEntity();
			emg->AddComponent<MeshAssetComponent>(meshAssetEntity, meshAsset);
			emg->AddComponent<MaterialRefComponent>(meshAssetEntity, matEnts[i]);
			if (opts.BuildBVH) {
				// Make these multithreaded some day
				BVHi* bvh{ nullptr };
				{
					ScopedTimer timer("Building BVH for mesh: \"" + meshAsset->GetName() + "\"");
					const auto& bc = emg->AddComponent<_BVHComponent>(meshAssetEntity, meshAsset->GetMesh());
					bvh = bc.bvh.get();
				}
				auto view = emg->GetView<SceneBVHComponent>();
				AHO_CORE_ASSERT(view.size() <= 1);
				view.each(
					[&](auto entity, SceneBVHComponent& sbc) {
						sbc.bvh->AddBLASPrimtive(bvh);
					}
				);
			}
		}
		m_Assets[opts.path] = firstMeshAsset;
		return firstMeshAsset;
	}

	std::shared_ptr<ShaderAsset> AssetManager::_LoadShaderAsset(const std::shared_ptr<EntityManager>& emg, const ShaderOptions& opts) {
		if (m_Assets.count(opts.path)) {
			return std::static_pointer_cast<ShaderAsset>(m_Assets.at(opts.path));
		}
		auto shaderAssetEntity = emg->CreateEntity();
		auto& shaderAssetComp = emg->AddComponent<ShaderAssetComponent>(shaderAssetEntity, std::make_shared<ShaderAsset>(opts.path, opts.usage));
		auto shaderAsset = shaderAssetComp.asset;

		m_Filewatcher.WatchFile(opts.path,
			[this, shaderAsset]() {
				AHO_CORE_TRACE("Reloading shader asset from path: `{}`", shaderAsset->GetPath());
				shaderAsset->Load();

				MainThreadDispatcher::Get().Enqueue([shaderAsset]() {
					// Dispatch the event on the main thread to ensure thread safety
					EngineEvents::OnShaderAssetReload.Dispatch(shaderAsset->GetPath(), shaderAsset.get());
				});
			});
		m_Filewatcher.Start();
		
		m_Assets[opts.path] = shaderAsset;
		return shaderAsset;
	}

}
