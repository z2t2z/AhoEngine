#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <unordered_map>

namespace Aho {
	LevelLayer::LevelLayer(RenderLayer* renderLayer, ResourceLayer* resourceLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("LevelLayer"), m_RenderLayer(renderLayer), m_ResourceLayer(resourceLayer), m_EventManager(eventManager), m_CameraManager(cameraManager) {
	}

	void LevelLayer::OnAttach() {
		m_CurrentLevel = std::make_shared<Level>();
		m_Levels.push_back(m_CurrentLevel);
	}
	
	void LevelLayer::OnDetach() {

	}
	
	void LevelLayer::OnUpdate(float deltaTime) {
		// UpdatePhysics();
		// UpdateAnimation();
		UpdataUBOData();
	}
	
	void LevelLayer::OnImGuiRender() {
	}
	
	void LevelLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::PackRenderData) {
			auto rawData = ((PackRenderDataEvent*)&e)->GetRawData();
			AHO_CORE_WARN("Recieving a PackRenderDataEvent!");
			LoadStaticMeshAsset(rawData);
			e.SetHandled();
		}
		if (e.GetEventType() == EventType::AddEntity) {
			auto type = ((AddLightSourceEvent*)&e)->GetLightType();
			AHO_CORE_WARN("Recieing a AddLightSourceEvent!");
			AddLightSource(type);
			e.Handled();
		}
	}

	void LevelLayer::UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll) {
		std::shared_ptr<UploadRenderDataEvent> newEv = std::make_shared<UploadRenderDataEvent>(renderDataAll);
		AHO_CORE_WARN("Pushing a UploadRenderDataEvent!");
		m_EventManager->PushBack(newEv);
	}

	void LevelLayer::AddLightSource(LightType lt) {
		if (m_LightData.lightCnt == MAX_LIGHT_CNT) {
			AHO_CORE_WARN("Maximum light count reached!");
			return;
		}
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto gameObject = entityManager->CreateEntity("PointLight");
		auto& pc = entityManager->AddComponent<PointLightComponent>(gameObject);
		pc.count = m_LightData.lightCnt++;
		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		for (const auto& meshInfo : *m_ResourceLayer->GetCube()) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			auto& tc = entityManager->AddComponent<TransformComponent>(gameObject, renderData->GetTransformParam());
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();
			mat->SetUniform("u_EntityID", entityID);	// TODO: should not be inside material
			renderData->SetMaterial(mat);
			renderData->SetVirtual();
			renderDataAll.push_back(renderData);
		}
		UploadRenderDataEventTrigger(renderDataAll);
	}

	void LevelLayer::UpdataUBOData() {
		// TODO: better way
		const auto& cam = m_CameraManager->GetMainEditorCamera();
		auto pipeline = m_RenderLayer->GetRenderer()->GetCurrentRenderPipeline();
		UBO* ubo = static_cast<UBO*>(pipeline->GetUBO(0));
		ubo->u_View = cam->GetView();
		ubo->u_Projection = cam->GetProjection();
		ubo->u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);

		GeneralUBO* gubo = static_cast<GeneralUBO*>(pipeline->GetUBO(1));
		gubo->u_View = cam->GetView();
		gubo->u_Projection = cam->GetProjection();
		gubo->u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);
		if (m_Update) {
			float nearPlane = 0.1f, farPlane = 85.0f;
			glm::mat4 proj = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, nearPlane, farPlane);
			auto lightMat = proj * glm::lookAt(glm::vec3(gubo->u_LightPosition[0]), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			ubo->u_LightViewMatrix = lightMat;
			gubo->u_LightViewMatrix = lightMat;
		}
		gubo->info[0] = m_LightData.lightCnt;
		for (int i = 0; i < m_LightData.lightCnt; i++) {
			gubo->u_LightPosition[i] = m_LightData.lightPosition[i];
			gubo->u_LightColor[i] = m_LightData.lightColor[i];
		}

		SSAOUBO* subo = static_cast<SSAOUBO*>(pipeline->GetUBO(2));
		subo->u_Projection = cam->GetProjection();
		subo->info[0] = pipeline->GetRenderPassTarget(RenderPassType::Final)->GetSpecification().Width;
		subo->info[1] = pipeline->GetRenderPassTarget(RenderPassType::Final)->GetSpecification().Height;
	}

	void LevelLayer::LoadStaticMeshAsset(std::shared_ptr<StaticMesh> rawData) {
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
			mat->SetUniform("u_EntityID", entityID);	// TODO: should not be inside material
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