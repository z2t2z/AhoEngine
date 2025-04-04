#include "Ahopch.h"
#include "LevelLayer.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Resource/Asset/Animation/Animator.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Texture.h"
#include "Runtime/Function/Gameplay/IK.h"
#include "Runtime/Core/BVH.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include <unordered_map>


#include <Jolt/Jolt.h>
// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace Aho {
	static BoneNode* g_Node;
	static BoneNode* g_Endeffector;
	static TransformParam* g_Param;
	LevelLayer::LevelLayer(RenderLayer* renderLayer, ResourceLayer* resourceLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("LevelLayer"), m_RenderLayer(renderLayer), m_ResourceLayer(resourceLayer), m_EventManager(eventManager), m_CameraManager(cameraManager) {
	}

	void LevelLayer::OnAttach() {
		m_CurrentLevel = std::make_shared<Level>();
		m_Levels.push_back(m_CurrentLevel);

		//JPH::RegisterTypes();
		//JPH::Factory::sInstance = new JPH::Factory();
		//JPH::RegisterDefaultAllocator();


	}

	void LevelLayer::OnDetach() {
	}

	void LevelLayer::OnUpdate(float deltaTime) {
		// UpdatePhysics();
		UpdateAnimation(deltaTime);
		UpdataUBOData();
		if (m_RenderLayer->GetRenderer()->GetRenderMode() == RenderMode::PathTracing) {
			UpdateSceneBvh();
		}
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
		if (e.GetEventType() == EventType::AddLight) {
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

	void LevelLayer::UpdatePathTracingTextureHandlesSSBO() {
		SSBOManager::UpdateSSBOData<TextureHandles>(5, m_TextureHandles);
	}

	void LevelLayer::UpdateAnimation(float deltaTime) {
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<AnimatorComponent, SkeletalComponent, AnimationComponent, SkeletonViewerComponent>();
		view.each([deltaTime, this](auto entity, auto& animator, auto& skeletal, auto& animation, auto& viewer) {
			animator.currentTime += deltaTime * animation.animation->GetTicksPerSecond();
			animator.currentTime = fmod(animator.currentTime, animation.animation->GetDuration());
			Animator::Update(animator.currentTime, animator.globalMatrices, skeletal.root, animation.animation, viewer.viewer);

			//IKSolver::FABRIK(viewer.viewer, animator.globalMatrices, g_Node, g_Endeffector, g_Param->Translation);
			Animator::ApplyPose(animator.globalMatrices, skeletal.root);
		});
	}

	void LevelLayer::UpdateSceneBvh() {
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<TransformComponent, BVHComponent>();
		bool gDirty = false;
		view.each(
			[&](auto entity, TransformComponent& transformComp, BVHComponent& bvhComp) {
				//if (transformComp.dirty) {
				//	gDirty = true;
				//	for (auto& bvh : bvhComp.bvhs) {
				//		bvh.ApplyTransform(transformComp.GetTransform());
				//	}
				//}
			}
		);

		if (gDirty) {
			
		}
	}

	// TODO;
	void LevelLayer::AddAnimation(const std::shared_ptr<AnimationAsset>& anim) {
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<SkeletalComponent, TransformComponent>();
		view.each([entityManager, anim, this](auto entity, auto& skeletal, auto& transformComponent) {
			if (!entityManager->HasComponent<AnimatorComponent>(entity)) {
				SkeletonViewer* viewer = new SkeletonViewer(skeletal.root);
				auto& animatorComponent = entityManager->AddComponent<AnimatorComponent>(entity, anim->GetBoneCnt());
				animatorComponent.boneOffset = AnimatorComponent::s_BoneOffset;
				AnimatorComponent::s_BoneOffset += anim->GetBoneCnt();
				entityManager->AddComponent<AnimationComponent>(entity, anim);
				entityManager->AddComponent<SkeletonViewerComponent>(entity, viewer);

				auto& entityComponent = entityManager->GetComponent<RootComponent>(entity); // store all bone entities
				// Adding all its bones to the ecs system
				const auto& transformMap = viewer->GetTransformMap();
				const auto& nodeIndexMap = viewer->GetBoneNodeIndexMap();
				AHO_CORE_ASSERT(transformMap.size() == nodeIndexMap.size());
				for (const auto& [node, transform] : transformMap) {
					//if (node->bone.name.find("LeftShoulder") != std::string::npos) {
					//	g_Node = node;
					//}
					//if (node->bone.name.find("LeftHandIndex1") != std::string::npos) {
					//	g_Endeffector = node;
					//}
					auto boneEntity = entityManager->CreateEntity();
					entityManager->AddComponent<TransformComponent>(boneEntity, node->transformParam); // Note that adding the delta transform
					entityComponent.entities.push_back(boneEntity.GetEntityHandle());
				}

				// Also upload a bone render data set as debug for skeleton visualization
				std::vector<std::shared_ptr<RenderData>> renderDataAll;
				for (const auto& meshInfo : *m_ResourceLayer->GetBone()) {
					std::shared_ptr<VertexArray> vao;
					vao.reset(VertexArray::Create());
					vao->Init(meshInfo);
					std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
					renderData->SetVAOs(vao);
					renderData->SetDebug();
					renderData->SetInstanced();
					vao->SetInstancedAmount(transformMap.size());
					renderData->SetTransformParam(transformComponent.transformPara);
					renderDataAll.push_back(renderData);
				}
				UploadRenderDataEventTrigger(renderDataAll);
			}
			});

	}

	void LevelLayer::AddLightSource(LightType lt) {
		auto entityManager = m_CurrentLevel->GetEntityManager();
		auto view = entityManager->GetView<PointLightComponent>();
		auto gameObject = entityManager->CreateEntity("PointLight " + std::to_string(view.size()));
		auto& pc = entityManager->AddComponent<PointLightComponent>(gameObject);
		pc.index = PointLightComponent::s_PointLightCnt++;
		if (pc.index == 0) {
			// Only the first light can cast shadow now
			pc.castShadow = true;
		}

		TransformParam* param = new TransformParam();
		param->Translation = glm::vec3(0.0f, 1.0f, 0.0f);
		g_Param = param;
		entityManager->AddComponent<TransformComponent>(gameObject, param); // transform component handle the the transformparam's lifetime
		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		for (const auto& meshInfo : *m_ResourceLayer->GetSphere()) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();
			renderData->SetTransformParam(param);
			renderData->SetDebug(true);
			renderData->SetEntityID(entityID);
			renderDataAll.push_back(renderData);
		}
		UploadRenderDataEventTrigger(renderDataAll);
	}

	// TODO: Subdata update
	// TODO: Maybe put this inside render layer
	void LevelLayer::UpdataUBOData() {
		const auto& cam = m_CameraManager->GetMainEditorCamera();
		{
			CameraUBO camUBO;
			camUBO.u_View = cam->GetView();
			camUBO.u_Projection = cam->GetProjection();
			camUBO.u_ProjectionInv = cam->GetProjectionInv();
			camUBO.u_ViewInv = cam->GetViewInv();
			camUBO.u_ViewProj = cam->GetView() * cam->GetProjection();
			camUBO.u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);
			UBOManager::UpdateUBOData<CameraUBO>(0, camUBO);
		}

		auto entityManager = m_CurrentLevel->GetEntityManager();
		{
			LightUBO lightUBO;
			auto view = entityManager->GetView<PointLightComponent, TransformComponent>();
			view.each([&](auto entity, auto& pc, auto& tc) {
				int index = pc.index;
				if (pc.castShadow) {
					float nearPlane = 0.1f, farPlane = 100.0f;
					glm::mat4 proj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, nearPlane, farPlane);
					auto lightMat = proj * glm::lookAt(glm::vec3(tc.GetTranslation()), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Only support one light that can cast shadow now
					lightUBO.u_LightPV[index] = lightMat;
					tc.GetTranslation().y = 20.0f;
				}
				lightUBO.u_Info[index].x = 1; // Flag: enabled or not
				lightUBO.u_LightPosition[index] = glm::vec4(tc.GetTranslation(), 1.0f);
				lightUBO.u_LightColor[index] = pc.color;
				});
			UBOManager::UpdateUBOData<LightUBO>(1, lightUBO);
		}

		{
			auto view = entityManager->GetView<AnimatorComponent, SkeletalComponent, AnimationComponent, SkeletonViewerComponent>();
			AnimationUBO* animationUBO = new AnimationUBO();
			view.each([&](auto entity, auto& animator, auto& skeletal, auto& anim, auto& viewer) {
				const BoneNode* root = skeletal.root;
				const std::vector<glm::mat4>& globalMatrices = animator.globalMatrices;
				int offset = animator.boneOffset;
				for (size_t i = 0; i < globalMatrices.size(); i++) {
					animationUBO->u_BoneMatrices[i + offset] = globalMatrices[i];
				}
				});
			UBOManager::UpdateUBOData<AnimationUBO>(3, *animationUBO);
			delete animationUBO;
		}

		{
			SkeletonUBO* skeletonUBO = new SkeletonUBO();
			auto view = entityManager->GetView<SkeletonViewerComponent, AnimatorComponent, RootComponent>();
			view.each([&](auto entity, auto& viewerComponent, auto& animatorComponent, auto& entityComponent) {
				const auto& boneTransform = viewerComponent.viewer->GetBoneTransform();
				const auto& transformMap = viewerComponent.viewer->GetTransformMap();
				const auto& nodeIndexMap = viewerComponent.viewer->GetBoneNodeIndexMap();
				int offset = animatorComponent.boneOffset;
				const auto& entities = entityComponent.entities;
				AHO_CORE_ASSERT(entities.size() == transformMap.size());
				size_t i = 0;
				for (const auto& [node, transform] : transformMap) {
					skeletonUBO->u_BoneMatrices[i] = boneTransform[nodeIndexMap.at(node)];
					skeletonUBO->u_BoneEntityID[i].x = static_cast<uint32_t>(entities[i]);
					i += 1;
				}
				});
			UBOManager::UpdateUBOData<SkeletonUBO>(4, *skeletonUBO);
			delete skeletonUBO;
		}
	}


	// Needs refactoring
	void LevelLayer::LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset) {
		auto entityManager = m_CurrentLevel->GetEntityManager();

		Entity gameObject(entityManager->CreateEntity(asset->GetName())); // TODO: give it a proper name
		entityManager->AddComponent<RootComponent>(gameObject);
		TransformParam* param = new TransformParam();
		entityManager->AddComponent<TransformComponent>(gameObject, param);

		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		renderDataAll.reserve(asset->size());
		int index = 0;

		for (const auto& meshInfo : *asset) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(meshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			auto meshEntity = entityManager->CreateEntity(asset->GetName() + "_" + std::to_string(index++));
			//entityManager->AddComponent<MeshComponent>(meshEntity, s_globalSubMeshId);
			entityManager->AddComponent<TransformComponent>(meshEntity, param);
			renderData->SetTransformParam(param);
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
			renderData->SetEntityID(entityID);
			renderData->SetMaterial(mat);

			m_MatMaskEnums.emplace_back(MaterialMaskEnum());
			MaterialMaskEnum& materialMask = m_MatMaskEnums.back();
			materialMask = MaterialMaskEnum::All;
			m_TextureHandles.emplace_back(TextureHandles());
			TextureHandles& handle = m_TextureHandles.back();

			entityManager->AddComponent<MaterialComponent>(meshEntity, mat, &handle, &materialMask);

			// should not be here
			if (meshInfo->materialInfo.HasMaterial()) {
				for (const auto& [type, path] : meshInfo->materialInfo.materials) {
					if (!textureCached.contains(path)) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
						tex->SetTexType(type);
						handle.SetHandles(tex->GetTextureHandle(), type);
						AHO_CORE_WARN("HandleId: {}", tex->GetTextureHandle());
						textureCached[path] = tex;
					}
					mat->AddMaterialProperties({ textureCached.at(path), type });
				}
			}
			if (!mat->HasProperty(TexType::Albedo)) {
				mat->AddMaterialProperties({ glm::vec3(0.95f), TexType::Albedo });
				materialMask ^= MaterialMaskEnum::AlbedoMap;
			}

			if (!mat->HasProperty(TexType::Normal)) {
				materialMask ^= MaterialMaskEnum::NormalMap;
			}

			if (!mat->HasProperty(TexType::Metallic)) {
				mat->AddMaterialProperties({ 0.2f, TexType::Metallic });
				materialMask ^= MaterialMaskEnum::MetallicMap;
			}

			if (!mat->HasProperty(TexType::Roughness)) {
				mat->AddMaterialProperties({ 0.90f, TexType::Roughness });
				materialMask ^= MaterialMaskEnum::RoughnessMap;
			}

			if (!mat->HasProperty(TexType::AO)) {
				mat->AddMaterialProperties({ 0.2f, TexType::AO });
				materialMask ^= MaterialMaskEnum::AOMap;
			}

			//tmp.push_back(materialMask);
			//entityManager->AddComponent<TextureHandlesComponent>(meshEntity, &handle, &materialMask);


			renderDataAll.push_back(renderData);
			entityManager->GetComponent<RootComponent>(gameObject).entities.push_back(meshEntity.GetEntityHandle());
		}


		{
			ScopedTimer timer("Build BVH");
			BVHi& tlasBvh = m_CurrentLevel->GetTLAS();
			BVHComponent& bvhComp = entityManager->AddComponent<BVHComponent>(gameObject);
			for (const auto& info : *asset) {
				assert(s_globalSubMeshId < m_MatMaskEnums.size());
				//BVHi* bvhi = new BVHi(info, s_globalSubMeshId++, m_MatMaskEnums[s_globalSubMeshId - 1]); // bad
				//BVHi* bvhi = new BVHi(info, s_globalSubMeshId, m_MatMaskEnums[s_globalSubMeshId++]); // bad
				BVHi* bvhi = new BVHi(info, s_globalSubMeshId, m_MatMaskEnums[s_globalSubMeshId]); // good
				s_globalSubMeshId += 1;

				bvhComp.bvhs.push_back(bvhi);
				tlasBvh.AddBLASPrimtive(bvhi);
			}
			tlasBvh.UpdateTLAS();
			PathTracingPipeline* ptpl = static_cast<PathTracingPipeline*>(m_RenderLayer->GetRenderer()->GetPipeline(RenderPipelineType::RPL_PathTracing));
			ptpl->UpdateSSBO(m_CurrentLevel);
			//std::thread(
			//	[this, gameObject, asset, entityManager]() {
			//		ScopedTimer timer("Build BVH(thread)");
			//		BVHi& tlasBvh = m_CurrentLevel->GetTLAS();
			//		BVHComponent& bvhComp = entityManager->AddComponent<BVHComponent>(gameObject);
			//		for (const auto& info : *asset) {
			//			auto bvhi = new BVHi(info, s_MeshCnt);
			//			bvhComp.bvhs.push_back(bvhi);
			//			tlasBvh.AddBLASNode(*bvhi);
			//		}
			//		tlasBvh.UpdateTLAS();
			//	}).detach();
		}

		UpdatePathTracingTextureHandlesSSBO();
		//asset.reset();
		UploadRenderDataEventTrigger(renderDataAll);
	}

	// Assume skeletal mesh is a whole mesh
	void LevelLayer::LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset) {
		auto entityManager = m_CurrentLevel->GetEntityManager();

		Entity gameObject(entityManager->CreateEntity(asset->GetName())); // TODO: give it a proper name
		//entityManager->AddComponent<MeshComponent>(gameObject, s_globalSubMeshId);
		entityManager->AddComponent<RootComponent>(gameObject);
		TransformParam* param = new TransformParam();
		entityManager->AddComponent<TransformComponent>(gameObject, param);
		auto& skeletalComponent = entityManager->AddComponent<SkeletalComponent>(gameObject, asset->GetRoot(), m_SkeletalMeshBoneOffset);

		m_SkeletalMeshBoneOffset += asset->GetBoneCnt();
		uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();
		std::vector<std::shared_ptr<RenderData>> renderDataAll;
		std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		renderDataAll.reserve(asset->size());
		int index = 0;
		for (const auto& skMeshInfo : *asset) {
			std::shared_ptr<VertexArray> vao;
			vao.reset(VertexArray::Create());
			vao->Init(skMeshInfo);
			std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
			renderData->SetVAOs(vao);
			std::shared_ptr<Material> mat = std::make_shared<Material>();
			renderData->SetEntityID(entityID);
			renderData->GetBoneOffset() = skeletalComponent.offset;
			renderData->SetMaterial(mat);
			if (skMeshInfo->materialInfo.HasMaterial()) {
				for (const auto& [type, path] : skMeshInfo->materialInfo.materials) {
					if (!textureCached.contains(path)) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
						tex->SetTexType(type);
						textureCached[path] = tex;
					}
					mat->AddMaterialProperties({ textureCached.at(path), type });
				}
			}
			if (!mat->HasProperty(TexType::Albedo)) {
				mat->AddMaterialProperties({ glm::vec3(0.95f), TexType::Albedo });
			}
			renderData->SetTransformParam(param);
			renderDataAll.push_back(renderData);
		}
		
		asset.reset();
		UploadRenderDataEventTrigger(renderDataAll);
	}
} // namespace Aho