#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Renderer/RenderData.h"


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
			e.SetHandled();
			auto ee = (PackRenderDataEvent*)&(e);
			AHO_CORE_WARN("Recieving a PackRenderDataEvent!");
			auto rawData = ee->GetRawData();

			if (!m_CurrentLevel) {
				m_CurrentLevel = std::make_shared<Level>();
				m_Levels.push_back(m_CurrentLevel);
			}

			Entity gameObject(m_CurrentLevel->GetEntityManager()->CreateEntity(ee->GetName()));
			m_CurrentLevel->GetEntityManager()->AddComponent<MultiMeshComponent>(gameObject);
			m_CurrentLevel->GetEntityManager()->AddComponent<EntityComponent>(gameObject);

			std::vector<std::shared_ptr<RenderData>> renderDataAll;
			for (const auto& meshInfo : *rawData) {
				std::shared_ptr<VertexArray> vao;
				vao.reset(VertexArray::Create());
				vao->Init(meshInfo);
				std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
				renderData->SetVAOs(vao);

				auto meshEntity = m_CurrentLevel->GetEntityManager()->CreateEntity("subMesh");
				m_CurrentLevel->GetEntityManager()->AddComponent<MeshComponent>(meshEntity, vao, static_cast<uint32_t>(meshEntity.GetEntityHandle()));
				m_CurrentLevel->GetEntityManager()->AddComponent<TransformComponent>(meshEntity);
				std::shared_ptr<Material> mat = std::make_shared<Material>();
				uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
				mat->SetUniform("u_EntityID", entityID); // setting entity id here
				if (meshInfo->materialInfo.HasMaterial()) {
					auto matEntity = m_CurrentLevel->GetEntityManager()->CreateEntity("subMesh");
					for (const auto& albedo : meshInfo->materialInfo.Albedo) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(albedo);
						tex->SetTextureType(TextureType::Diffuse);
						mat->AddTexture(tex);
						if (tex->IsLoaded()) {
							// TODO: Cache the loaded texture
						}
					}
					for (const auto& normal : meshInfo->materialInfo.Normal) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(normal);
						tex->SetTextureType(TextureType::Normal);
						mat->AddTexture(tex);
						if (tex->IsLoaded()) {

						}
					}
					renderData->SetMaterial(mat);
					m_CurrentLevel->GetEntityManager()->AddComponent<MaterialComponent>(meshEntity, mat);
				}
				renderDataAll.push_back(renderData);
				m_CurrentLevel->GetEntityManager()->GetComponent<EntityComponent>(gameObject).meshEntities.push_back(meshEntity.GetEntityHandle());
			}
			/* Check if success */
			UploadRenderDataEventTrigger(renderDataAll);
		}
	}

	void LevelLayer::UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll) {
		std::shared_ptr<UploadRenderDataEvent> newEv = std::make_shared<UploadRenderDataEvent>(renderDataAll);
		AHO_CORE_WARN("Pushing a UploadRenderDataEvent!");
		m_EventManager->PushBack(newEv);
	}
} // namespace Aho