#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/Asset/Animation/Animator.h"
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
		UpdataUBOData();
		// UpdatePhysics();
		UpdateAnimation(deltaTime);
	}
	
	void LevelLayer::OnImGuiRender() {
	}
	
	void LevelLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::PackRenderData) {
			bool isSkeletal = ((PackRenderDataEvent*)&e)->IsSkeletalMesh();
			auto rawData = ((PackRenderDataEvent*)&e)->GetRawData();
			AHO_CORE_WARN("Recieving a PackRenderDataEvent!");
			isSkeletal ? LoadSkeletalMeshAsset(static_pointer_cast<SkeletalMesh>(rawData)) : LoadStaticMeshAsset(static_pointer_cast<StaticMesh>(rawData));
			e.SetHandled();
		}
		if (e.GetEventType() == EventType::AddEntity) {
			auto type = ((AddLightSourceEvent*)&e)->GetLightType();
			AHO_CORE_WARN("Recieing a AddLightSourceEvent!");
			AddLightSource(type);
			e.Handled();
		}
		if (e.GetEventType() == EventType::AddAnimation) {
			auto anim = ((UploadAnimationDataEvent*)&e)->GetAnimationAssetData();
			AddAnimation(anim);
			e.Handled();
		}
	}

	void LevelLayer::UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll) {
		std::shared_ptr<UploadRenderDataEvent> newEv = std::make_shared<UploadRenderDataEvent>(renderDataAll);
		AHO_CORE_WARN("Pushing a UploadRenderDataEvent!");
		m_EventManager->PushBack(newEv);
	}

	void LevelLayer::UpdateAnimation(float deltaTime) {
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<AnimatorComponent, SkeletalComponent, AnimationComponent>();
		view.each([deltaTime, this](auto entity, auto& animator, auto& skeletal, auto& animation) {
			const BoneNode* root = skeletal.root;
			std::vector<glm::mat4>& globalMatrices = animator.globalMatrices;
			float& currentTime = animator.currentTime;
			const std::shared_ptr<AnimationAsset> anim = animation.animation;
			currentTime += deltaTime * anim->GetTicksPerSecond();
			currentTime = fmod(currentTime, anim->GetDuration());
			Animator::Update(currentTime, globalMatrices, root, anim);

			auto pipeline = m_RenderLayer->GetRenderer()->GetCurrentRenderPipeline();
			SkeletalUBO* skubo = static_cast<SkeletalUBO*>(pipeline->GetUBO(3));
			for (size_t i = 0; i < globalMatrices.size(); i++) {
				skubo->u_BoneMatrices[i] = globalMatrices[i];
			}
		});
	}

	void LevelLayer::AddAnimation(const std::shared_ptr<AnimationAsset>& anim) {
		AHO_CORE_TRACE("Adding animation");
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<SkeletalComponent>();
		view.each([entityManager, anim](auto entity, auto& skeletal) {
			entityManager->AddComponent<AnimatorComponent>(entity, anim->GetBoneCnt());
			entityManager->AddComponent<AnimationComponent>(entity, anim);
		});
	}

	void LevelLayer::AddLightSource(LightType lt) {
		if (m_LightData.lightCnt == MAX_LIGHT_CNT) {
			AHO_CORE_WARN("Maximum light count reached!");
			return;
		}
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto gameObject = entityManager->CreateEntity("PointLight");
		auto& pc = entityManager->AddComponent<PointLightComponent>(gameObject);
		TransformParam* param = new TransformParam();
		entityManager->AddComponent<TransformComponent>(gameObject, param); // transform component handle the the transformparam's lifetime
		pc.count = m_LightData.lightCnt++;
		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		for (const auto& meshInfo : *m_ResourceLayer->GetCube()) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();
			mat->SetUniform("u_EntityID", entityID);	// TODO: should not be inside material
			renderData->SetMaterial(mat);
			renderData->SetTransformParam(param);
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

		SkeletalUBO* skubo = static_cast<SkeletalUBO*>(pipeline->GetUBO(3));
		skubo->u_View = cam->GetView();
		skubo->u_Projection = cam->GetProjection();
		skubo->u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);

		if (m_Update) {
			float nearPlane = 0.1f, farPlane = 1000.0f;
			glm::mat4 proj = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
			auto lightMat = proj * glm::lookAt(glm::vec3(gubo->u_LightPosition[0]), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			ubo->u_LightPV = lightMat;
			gubo->u_LightPV = lightMat;
			skubo->u_LightPV = lightMat;
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

	void LevelLayer::LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset) {
		auto entityManager = m_CurrentLevel->GetEntityManager();

		Entity gameObject(entityManager->CreateEntity("StaticMesh")); // TODO: give it a proper name
		entityManager->AddComponent<MultiMeshComponent>(gameObject);
		entityManager->AddComponent<EntityComponent>(gameObject);

		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		renderDataAll.reserve(asset->size());
		for (const auto& meshInfo : *asset) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			auto meshEntity = entityManager->CreateEntity("subMesh");
			//entityManager->AddComponent<MeshComponent>(meshEntity, vao, static_cast<uint32_t>(meshEntity.GetEntityHandle()));
			entityManager->AddComponent<MeshComponent>(meshEntity);
			TransformParam* param = new TransformParam();
			entityManager->AddComponent<TransformComponent>(meshEntity, param);
			renderData->SetTransformParam(param);
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
		asset.reset(); // Free it??
		/* TODO: Maybe check if success */
		UploadRenderDataEventTrigger(renderDataAll);
	}

	// Assume skeletal mesh is a whole mesh
	void LevelLayer::LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset) {
		auto entityManager = m_CurrentLevel->GetEntityManager();

		Entity gameObject(entityManager->CreateEntity("SkeletalMesh")); // TODO: give it a proper name
		entityManager->AddComponent<MeshComponent>(gameObject);
		entityManager->AddComponent<EntityComponent>(gameObject);
		entityManager->AddComponent<SkeletalComponent>(gameObject, asset->GetRoot(), asset->GetBoneCache());

		m_SkeletonViewer = std::make_unique<SkeletonViewer>(asset->GetRoot());
		{
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(m_SkeletonViewer->GetVAO());
			renderData->SetLine();
			UploadRenderDataEventTrigger({ renderData});
		}


		TransformParam* param = new TransformParam();
		param->Scale = glm::vec3(0.01f, 0.01f, 0.01f);
		entityManager->AddComponent<TransformComponent>(gameObject, param);
		uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();

		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		renderDataAll.reserve(asset->size());
		if (asset->size() > 1) {
			AHO_CORE_WARN("Skeletal mesh has more than one sub meshes, combinng as one assembly");  // TODO: add a path
		}
		for (const auto& skMeshInfo : *asset) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(skMeshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			mat->SetUniform("u_EntityID", entityID);	// TODO: Should not be inside material
			mat->SetUniform("u_AO", 0.1f);				// TODO: Should not be done this way
			mat->SetUniform("u_Metalic", 0.2f);
			mat->SetUniform("u_Roughness", 0.2f);
			//entityManager->AddComponent<MaterialComponent>(gameObject, mat); // TODO;
			renderData->SetMaterial(mat);
			if (skMeshInfo->materialInfo.HasMaterial()) {
				auto matEntity = entityManager->CreateEntity("subMesh");
				for (const auto& [type, path] : skMeshInfo->materialInfo.materials) {
					if (!textureCached.contains(path)) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
						tex->SetTextureType(type);
						textureCached[path] = tex;
					}
					mat->AddTexture(textureCached.at(path));
				}
			}
			renderData->SetTransformParam(param);
			renderDataAll.push_back(renderData);
		}
		AHO_CORE_ASSERT(param, "Something wrong when initializing skeletal mesh's transform parameter!");
		//entityManager->AddComponent<TransformComponent>(gameObject, param);
		asset.reset();
		/* TODO: Maybe check if success */
		UploadRenderDataEventTrigger(renderDataAll);
	}
} // namespace Aho