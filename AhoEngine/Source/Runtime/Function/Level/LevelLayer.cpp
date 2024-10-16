#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <unordered_map>

namespace Aho {
	LevelLayer::LevelLayer(RenderLayer* renderLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: m_RenderLayer(renderLayer), m_EventManager(eventManager), m_CameraManager(cameraManager) {
	}

	void LevelLayer::OnAttach() {
		
	}
	
	void LevelLayer::OnDetach() {

	}
	
	void LevelLayer::OnUpdate(float deltaTime) {
		SubmitRenderData();
		// UpdatePhysics();
		// UpdateAnimation();
		/* Actual in-game logic here */
		if (!m_PlayMode) {
			return;
		}
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

	void LevelLayer::SubmitRenderData() {
		const auto& cam = m_CameraManager->GetMainEditorCamera();
		m_RenderLayer->GetUBO()->u_Projection = cam->GetProjection();
		m_RenderLayer->GetUBO()->u_View = cam->GetView();
		m_RenderLayer->GetUBO()->u_ViewPosition = glm::vec4(cam->GetPosition(), 0.0f);
		//for (int i = 0; i < 4; i++) {
		//	m_RenderLayer->GetUBO()->u_LightPosition[i] = m_LightData.lightPosition[i];
		//	m_RenderLayer->GetUBO()->u_LightColor[i] = m_LightData.lightColor[i];
		//}
		if (m_Update) {
			//m_RenderLayer->GetUBO()->u_LightViewMatrix = cam->GetProjection() * cam->GetView();
			float nearPlane = 0.1f, farPlane = 50.0f;
			glm::mat4 proj = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, nearPlane, farPlane);
			auto pos = glm::vec3(m_RenderLayer->GetUBO()->u_LightPosition[0]);
			m_RenderLayer->GetUBO()->u_LightViewMatrix = proj * glm::lookAt(glm::vec3(m_RenderLayer->GetUBO()->u_LightPosition[0]), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
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
		std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		renderDataAll.reserve(rawData->size());
		for (const auto& meshInfo : *rawData) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			auto meshEntity = entityManager->CreateEntity("subMesh");
			entityManager->AddComponent<MeshComponent>(meshEntity, vao, static_cast<uint32_t>(meshEntity.GetEntityHandle()));
			auto& tc = entityManager->AddComponent<TransformComponent>(meshEntity, renderData->GetTransformParam());
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
			mat->SetUniform("u_EntityID", entityID);	// setting entity id here
			mat->SetUniform("u_AO", 0.1f);
			mat->SetUniform("u_Metalic", 0.2f);
			mat->SetUniform("u_Roughness", 0.2f);
			entityManager->AddComponent<MaterialComponent>(meshEntity, mat);
			renderData->SetMaterial(mat);
			if (meshInfo->materialInfo.HasMaterial()) {
				auto matEntity = entityManager->CreateEntity("subMesh");
				for (const auto& [type, path] : meshInfo->materialInfo.materials) {
					if (!textureCached.contains(path)) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
						tex->SetTextureType(type);
						textureCached[path] = tex;
					}
					mat->AddTexture(textureCached.at(path));
				}
			}
			renderDataAll.push_back(renderData);
			entityManager->GetComponent<EntityComponent>(gameObject).entities.push_back(meshEntity.GetEntityHandle());
		}
		/* TODO: Maybe check if success */
		UploadRenderDataEventTrigger(renderDataAll);
	}

	void LevelLayer::LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset) {

	}
} // namespace Aho