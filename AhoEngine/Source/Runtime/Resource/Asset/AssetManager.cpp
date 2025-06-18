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
		std::vector<MaterialPaths> mats; // TODO
		bool success = AssetLoader::MeshLoader(opts, meshes, mats);
		AHO_CORE_ASSERT(success && meshes.size() == mats.size());

		std::vector<Entity> matEnts;
		matEnts.reserve(mats.size());
		for (const MaterialPaths& mat : mats) {
			std::shared_ptr<MaterialAsset> matAsset = std::make_shared<MaterialAsset>(opts.path, mat.UsagePaths);
			matEnts.push_back(emg->CreateEntity());
			emg->AddComponent<MaterialAssetComponent>(matEnts.back(), matAsset);
		}

		std::shared_ptr<MeshAsset> firstMeshAsset; 
		std::vector<Entity> meshAssetEntities; meshAssetEntities.reserve(meshes.size());
		for (size_t i = 0; i < meshes.size(); i++) {
			std::shared_ptr<MeshAsset> meshAsset = std::make_shared<MeshAsset>(opts.path, meshes[i].name, meshes[i]);
			if (i == 0) {
				firstMeshAsset = meshAsset;
			}
			Entity meshAssetEntity = emg->CreateEntity();
			emg->AddComponent<MeshAssetComponent>(meshAssetEntity, meshAsset);
			emg->AddComponent<MaterialRefComponent>(meshAssetEntity, matEnts[i]);
			meshAssetEntities.push_back(meshAssetEntity);
		}

		if (opts.BuildBVH) {
			// Add a TLAS bvh first
			if (opts.BuildBVH) {
				if (emg->GetView<SceneBVHComponent>().empty()) {
					auto scene = emg->CreateEntity();
					emg->AddComponent<SceneBVHComponent>(scene);
				}
			}

			std::vector<std::shared_ptr<BVHi>> bvhs; bvhs.reserve(meshes.size());
			auto view = emg->GetView<SceneBVHComponent>();
			AHO_CORE_ASSERT(view.size() <= 1);
			Entity sceneBVHEntity;
			size_t offset = 0;
			view.each(
				[&sceneBVHEntity, &offset](Entity entity, SceneBVHComponent& sbc) { 
					sceneBVHEntity = entity; 
					offset = sbc.bvh->GetPrimsCount(); 
				});

			size_t siz = meshes.size();
			bvhs.resize(siz);
			
			g_RuntimeGlobalCtx.m_ParallelExecutor->ParallelFor(siz,
				[&meshes, &bvhs, offset](int64_t i) {
					ScopedTimer timer("Building BVH");
					bvhs[i] = std::make_shared<BVHi>(meshes[i], i + offset);
				}, 1);
			
			for (size_t i = 0; i < siz; i++) {
				const auto& bc = emg->AddComponent<_BVHComponent>(meshAssetEntities[i], bvhs[i]);
				view.each(
					[&](auto entity, SceneBVHComponent& sbc) {
						sbc.bvh->AddBLASPrimtive(bc.bvh.get());
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
				AHO_CORE_TRACE("AssetManager::Reloading shader asset from path: `{}`", shaderAsset->GetPath());
				shaderAsset->Load();

				MainThreadDispatcher::Get().Enqueue([shaderAsset]() {
					// Dispatch the event on the main thread to ensure thread safety
					EngineEvents::OnShaderAssetReload.Dispatch(shaderAsset->GetPath(), shaderAsset);
				});
			});
		m_Filewatcher.Start();
		
		m_Assets[opts.path] = shaderAsset;
		return shaderAsset;
	}

}
