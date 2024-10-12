#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include <mutex>


namespace Aho {
	LevelLayer::LevelLayer(EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: m_EventManager(eventManager), m_CameraManager(cameraManager) {
	}

	void LevelLayer::OnAttach() {
		
	}
	
	void LevelLayer::OnDetach() {

	}
	
	void LevelLayer::OnUpdate(float deltaTime) {
		if (!m_PlayMode) {
			return;
		}
		/* Actual in-game logic here */
	}
	
	void LevelLayer::OnImGuiRender() {
	}
	
	void LevelLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::PackRenderData) {
			auto ee = (PackRenderDataEvent*)&(e);
			AHO_CORE_WARN("Recieving a PackRenderDataEvent!");
			auto rawData = ee->GetRawData();
			LoadStaticMeshAsset(rawData);
			e.SetHandled();
		}
	}

	void LevelLayer::UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll) {
		std::shared_ptr<UploadRenderDataEvent> newEv = std::make_shared<UploadRenderDataEvent>(renderDataAll);
		AHO_CORE_WARN("Pushing a UploadRenderDataEvent!");
		m_EventManager->PushBack(newEv);
	}

	void LevelLayer::LoadStaticMeshAsset(std::shared_ptr<StaticMesh> rawData) {
		if (!m_CurrentLevel) {
			m_CurrentLevel = std::make_shared<Level>();
			m_Levels.push_back(m_CurrentLevel);
		}
		auto entityManager = m_CurrentLevel->GetEntityManager();

		Entity gameObject(entityManager->CreateEntity("StaticMesh")); // TODO: give it a proper name
		entityManager->AddComponent<MultiMeshComponent>(gameObject);
		entityManager->AddComponent<EntityComponent>(gameObject);

		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		for (const auto& meshInfo : *rawData) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);

			auto meshEntity = entityManager->CreateEntity("subMesh");
			entityManager->AddComponent<MeshComponent>(meshEntity, vao, static_cast<uint32_t>(meshEntity.GetEntityHandle()));
			entityManager->AddComponent<TransformComponent>(meshEntity);
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
			mat->SetUniform("u_EntityID", entityID); // setting entity id here
			if (meshInfo->materialInfo.HasMaterial()) {
				auto matEntity = entityManager->CreateEntity("subMesh");
				for (const auto& albedo : meshInfo->materialInfo.Albedo) {
					std::shared_ptr<Texture2D> tex = Texture2D::Create(albedo); // TODO: should be done in the resource layer!
					tex->SetTextureType(TextureType::Diffuse);
					mat->AddTexture(tex);
					if (tex->IsLoaded()) {
						// TODO: Cache the loaded texture
					}
				}
				for (const auto& normal : meshInfo->materialInfo.Normal) {
					std::shared_ptr<Texture2D> tex = Texture2D::Create(normal); // TODO: should be done in the resource layer!
					tex->SetTextureType(TextureType::Normal);
					mat->AddTexture(tex);
					if (tex->IsLoaded()) {
						// TODO: Cache the loaded texture
					}
				}
				renderData->SetMaterial(mat);
				entityManager->AddComponent<MaterialComponent>(meshEntity, mat);
			}
			renderDataAll.push_back(renderData);
			entityManager->GetComponent<EntityComponent>(gameObject).meshEntities.push_back(meshEntity.GetEntityHandle());
		}
		/* TODO: Maybe check if success */
		UploadRenderDataEventTrigger(renderDataAll);
	}
} // namespace Aho