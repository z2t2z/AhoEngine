#include "Ahopch.h"

#include "LevelLayer.h"
#include "Level.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Resource/Asset/Animation/Animator.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Gameplay/IK.h"
#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Function/SkeletonViewer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/RenderLayer.h"

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
	LevelLayer::LevelLayer(RenderLayer* renderLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("LevelLayer"), m_RenderLayer(renderLayer), m_EventManager(eventManager), m_CameraManager(cameraManager) {
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
		UpdateAnimation(deltaTime);
		UpdataUBOData();
	}

	void LevelLayer::OnImGuiRender() {
	}

	void LevelLayer::OnEvent(Event& e) {
	}

	void LevelLayer::UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll) {
		std::shared_ptr<UploadRenderDataEvent> newEv = std::make_shared<UploadRenderDataEvent>(renderDataAll);
		AHO_CORE_WARN("Pushing a UploadRenderDataEvent!");
		m_EventManager->PushBack(newEv);
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
				//for (const auto& meshInfo : *m_ResourceLayer->GetBone()) {
				//	std::shared_ptr<VertexArray> vao;
				//	vao.reset(VertexArray::Create());
				//	vao->Init(meshInfo);
				//	std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
				//	renderData->SetVAOs(vao);
				//	renderData->SetDebug();
				//	renderData->SetInstanced();
				//	vao->SetInstancedAmount(transformMap.size());
				//	renderData->SetTransformParam(transformComponent.transformPara);
				//	renderDataAll.push_back(renderData);
				//}
				UploadRenderDataEventTrigger(renderDataAll);
			}
		});

	}

	// TODO: Subdata update
	// TODO: Maybe put this inside render layer
	void LevelLayer::UpdataUBOData() {
		return;

		const auto& cam = m_CameraManager->GetMainEditorCamera();
		CameraUBO camUBO;
		camUBO.u_View = cam->GetView();
		camUBO.u_Projection = cam->GetProjection();
		camUBO.u_ProjectionInv = cam->GetProjectionInv();
		camUBO.u_ViewInv = cam->GetViewInv();
		camUBO.u_ProjView = cam->GetProjection() * cam->GetView();
		camUBO.u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);
		UBOManager::UpdateUBOData<CameraUBO>(0, camUBO);
		return;

		auto entityManager = m_CurrentLevel->GetEntityManager();
		{
			LightUBO* lightubo = new LightUBO();
			{
				auto view = entityManager->GetView<SkyComponent>();
				view.each(
					[&](auto entity, auto& sky) {
						lightubo->u_LightCount.x += 1;
						float nearPlane = 0.1f, farPlane = 100.0f;
						glm::mat4 proj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
						static constexpr float s_sceneRadius = 10.0f;
						auto dir = sky.DirectionXYZ;
						auto lightMat = proj * glm::lookAt(dir * s_sceneRadius, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
						auto col = sky.Color;
						float intensity = sky.Intensity;
						lightubo->u_DirLight[0] = GPU_DirectionalLight(lightMat, dir, col, intensity);
					});
			}
			{
				auto view = entityManager->GetView<LightComponent, TransformComponent>();
				int areaLightCount = 0;
				view.each(
					[&](auto entity, auto& lc, auto& tc) {
						std::shared_ptr<Light> light = lc.light;
						switch (light->GetType()) {
							case LightType::Area: {
								lightubo->u_LightCount.w += 1;
								auto area = static_cast<AreaLight*>(light.get()); // Needs better way
								lightubo->u_AreaLight[areaLightCount++] = GPU_AreaLight(tc.GetTransform(), light->GetColor(), light->GetIntensity(), area->GetWidth(), area->GetHeight());
								break;
							}
							case LightType::Point: {
								AHO_CORE_ASSERT(false); // Should not happen
								break;
							}
							case LightType::Directional: {
								AHO_CORE_ASSERT(false);
								break;
							}
							case LightType::Spot: {
								AHO_CORE_ASSERT(false);
								break;
							}
							default: {
								AHO_CORE_ASSERT(false);
								break;
							}
						}
					});
			}
			UBOManager::UpdateUBOData<LightUBO>(1, *lightubo);
			delete lightubo;
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

	void LevelLayer::AddStaticMeshToScene(const std::shared_ptr<StaticMesh>& asset, const std::string& name, const std::shared_ptr<Light>& light) {
		//auto entityManager = m_CurrentLevel->GetEntityManager();

		////Entity gameObject = entityManager->CreateEntity(name); // TODO: give it a proper name
		//Entity gameObject = entityManager->CreateEntity(); // TODO: give it a proper name
		//entityManager->AddComponent<RootComponent>(gameObject);
		//TransformParam* param = new TransformParam();
		//entityManager->AddComponent<TransformComponent>(gameObject, param);
		//if (light) {
		//	entityManager->AddComponent<LightComponent>(gameObject, light);
		//}
		//std::vector<std::shared_ptr<RenderData>> renderDataAll;
		//std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		//renderDataAll.reserve(asset->size());
		//int index = 0;

		//for (const auto& meshInfo : *asset) {
		//	std::shared_ptr<VertexArray> vao;
		//	vao.reset(VertexArray::Create());
		//	vao->Init(meshInfo);
		//	std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
		//	renderData->SetVAOs(vao);
		//	auto meshEntity = entityManager->CreateEntity();
		//	entityManager->AddComponent<TransformComponent>(meshEntity, param);
		//	entityManager->AddComponent<MeshComponent>(meshEntity, renderData);

		//	renderData->SetTransformParam(param);
		//	std::shared_ptr<Material> mat = std::make_shared<Material>();
		//	uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
		//	renderData->SetEntityID(entityID);
		//	renderData->SetMaterial(mat);

		//	MaterialDescriptor handle;
		//	// should not be here
		//	if (meshInfo->materialInfo.HasMaterial()) {
		//		for (const auto& [type, path] : meshInfo->materialInfo.materials) {
		//			if (!textureCached.contains(path)) {
		//				std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
		//				tex->SetTexType(type);
		//				handle.SetHandles(tex->GetTextureHandle(), type);
		//				textureCached[path] = tex;
		//			}
		//			mat->AddMaterialProperties({ textureCached.at(path), type });
		//		}
		//	}

		//	// TODO: Ugly code, find a way to iterate these
		//	if (!mat->HasProperty(TexType::Albedo)) {
		//		mat->AddMaterialProperties({ glm::vec3(1.0f), TexType::Albedo });
		//		handle.SetValue(glm::vec3(1.0f), TexType::Albedo);
		//	}
		//	glm::vec3 color(0.0f);
		//	float intensity = 0.0f;
		//	if (light) {
		//		color = light->GetColor();
		//		intensity = light->GetIntensity();
		//	}
		//	if (!mat->HasProperty(TexType::Emissive)) {
		//		mat->AddMaterialProperties({ color, TexType::Emissive });
		//		handle.SetValue(color, TexType::Emissive);
		//	}
		//	if (!mat->HasProperty(TexType::EmissiveScale)) {
		//		mat->AddMaterialProperties({ intensity, TexType::EmissiveScale });
		//		handle.SetValue(intensity, TexType::EmissiveScale);
		//	}
		//	if (!mat->HasProperty(TexType::Normal)) {
		//		mat->AddMaterialProperties({ glm::vec3(0.0f), TexType::Normal });
		//	}
		//	if (!mat->HasProperty(TexType::Metallic)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Metallic });
		//		handle.SetValue(0.0f, TexType::Metallic);
		//	}
		//	if (!mat->HasProperty(TexType::Specular)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Specular });
		//		handle.SetValue(0.0f, TexType::Specular);
		//	}
		//	if (!mat->HasProperty(TexType::Roughness)) {
		//		mat->AddMaterialProperties({ 0.5f, TexType::Roughness });
		//		handle.SetValue(0.5f, TexType::Roughness);
		//	}
		//	if (!mat->HasProperty(TexType::Subsurface)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Subsurface });
		//		handle.SetValue(0.0f, TexType::Subsurface);
		//	}
		//	if (!mat->HasProperty(TexType::SpecTint)) {
		//		mat->AddMaterialProperties({ 0.00f, TexType::SpecTint });
		//		handle.SetValue(0.0f, TexType::SpecTint);
		//	}
		//	if (!mat->HasProperty(TexType::Anisotropic)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Anisotropic });
		//		handle.SetValue(0.0f, TexType::Anisotropic);
		//	}
		//	if (!mat->HasProperty(TexType::Sheen)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Sheen });
		//		handle.SetValue(0.0f, TexType::Sheen);
		//	}
		//	if (!mat->HasProperty(TexType::SheenTint)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::SheenTint });
		//		handle.SetValue(0.0f, TexType::SheenTint);
		//	}
		//	if (!mat->HasProperty(TexType::Clearcoat)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Clearcoat });
		//		handle.SetValue(0.0f, TexType::Clearcoat);
		//	}
		//	if (!mat->HasProperty(TexType::ClearcoatGloss)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::ClearcoatGloss });
		//		handle.SetValue(0.0f, TexType::ClearcoatGloss);
		//	}
		//	if (!mat->HasProperty(TexType::SpecTrans)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::SpecTrans });
		//		handle.SetValue(0.0f, TexType::SpecTrans);
		//	}
		//	if (!mat->HasProperty(TexType::ior)) {
		//		mat->AddMaterialProperties({ 1.5f, TexType::ior });
		//		handle.SetValue(1.5f, TexType::ior);
		//	}
		//	if (!mat->HasProperty(TexType::AO)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::AO });
		//	}
		//	auto& materialComp = entityManager->AddComponent<MaterialComponent>(meshEntity, mat, (int32_t)m_TextureHandles.size());
		//	m_TextureHandles.push_back(handle);
		//	renderDataAll.push_back(renderData);
		//	entityManager->GetComponent<RootComponent>(gameObject).entities.push_back(meshEntity.GetEntityHandle());
		//}

		//if (m_BuildBVH || light) {
		//	ScopedTimer timer("Build BVH");
		//	BVHi& tlasBvh = m_CurrentLevel->GetTLAS();
		//	BVHComponent& bvhComp = entityManager->AddComponent<BVHComponent>(gameObject);
		//	for (const auto& info : *asset) {
		//		BVHi* bvhi = new BVHi(info, s_globalSubMeshId++);
		//		bvhComp.bvhs.push_back(bvhi);
		//		tlasBvh.AddBLASPrimtive(bvhi);
		//	}
		//	tlasBvh.UpdateTLAS();
		//	//PathTracingPipeline* ptpl = static_cast<PathTracingPipeline*>(m_RenderLayer->GetRenderer()->GetPipeline(RenderPipelineType::RPL_PathTracing));
		//	//ptpl->UpdateSSBO(m_CurrentLevel);
		//}

		//UpdatePathTracingTextureHandlesSSBO();
		//UploadRenderDataEventTrigger(renderDataAll);
	}

	// Needs refactoring
	void LevelLayer::LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset) {
		//auto entityManager = m_CurrentLevel->GetEntityManager();

		//Entity gameObject(entityManager->CreateEntity()); // TODO: give it a proper name
		//entityManager->AddComponent<RootComponent>(gameObject);
		//TransformParam* param = new TransformParam();
		//entityManager->AddComponent<TransformComponent>(gameObject, param);

		//std::vector<std::shared_ptr<RenderData>> renderDataAll;
		//std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		//renderDataAll.reserve(asset->size());
		//int index = 0;

		//for (const auto& meshInfo : *asset) {
		//	std::shared_ptr<VertexArray> vao;
		//	vao.reset(VertexArray::Create());
		//	vao->Init(meshInfo);
		//	std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
		//	renderData->SetVAOs(vao);
		//	auto meshEntity = entityManager->CreateEntity();
		//	entityManager->AddComponent<TransformComponent>(meshEntity, param);
		//	entityManager->AddComponent<MeshComponent>(meshEntity, renderData);

		//	renderData->SetTransformParam(param);
		//	std::shared_ptr<Material> mat = std::make_shared<Material>();
		//	uint32_t entityID = (uint32_t)meshEntity.GetEntityHandle();
		//	renderData->SetEntityID(entityID);
		//	renderData->SetMaterial(mat);

		//	MaterialDescriptor handle;
		//	// should not be here
		//	if (meshInfo->materialInfo.HasMaterial()) {
		//		for (const auto& [type, path] : meshInfo->materialInfo.materials) {
		//			if (!textureCached.contains(path)) {
		//				std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
		//				tex->SetTexType(type);
		//				handle.SetHandles(tex->GetTextureHandle(), type);
		//				textureCached[path] = tex;
		//			}
		//			mat->AddMaterialProperties({ textureCached.at(path), type });
		//		}
		//	}

		//	// TODO: Ugly code, find a way to iterate these
		//	if (!mat->HasProperty(TexType::Albedo)) {
		//		mat->AddMaterialProperties({ glm::vec3(0.95f), TexType::Albedo });
		//		handle.SetValue(glm::vec3(0.95), TexType::Albedo);
		//	}
		//	if (!mat->HasProperty(TexType::Emissive)) {
		//		mat->AddMaterialProperties({ glm::vec3(0.0), TexType::Emissive });
		//		handle.SetValue(glm::vec3(0.0), TexType::Emissive);
		//	}
		//	if (!mat->HasProperty(TexType::EmissiveScale)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::EmissiveScale });
		//		handle.SetValue(0.0f, TexType::EmissiveScale);
		//	}
		//	if (!mat->HasProperty(TexType::Normal)) {
		//		mat->AddMaterialProperties({ glm::vec3(0.0f), TexType::Normal });
		//	}
		//	if (!mat->HasProperty(TexType::Metallic)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Metallic });
		//		handle.SetValue(0.0f, TexType::Metallic);
		//	}
		//	if (!mat->HasProperty(TexType::Specular)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Specular });
		//		handle.SetValue(0.0f, TexType::Specular);
		//	}
		//	if (!mat->HasProperty(TexType::Roughness)) {
		//		mat->AddMaterialProperties({ 0.5f, TexType::Roughness });
		//		handle.SetValue(0.5f, TexType::Roughness);
		//	}
		//	if (!mat->HasProperty(TexType::Subsurface)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Subsurface });
		//		handle.SetValue(0.0f, TexType::Subsurface);
		//	}
		//	if (!mat->HasProperty(TexType::SpecTint)) {
		//		mat->AddMaterialProperties({ 0.00f, TexType::SpecTint });
		//		handle.SetValue(0.0f, TexType::SpecTint);
		//	}
		//	if (!mat->HasProperty(TexType::Anisotropic)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Anisotropic });
		//		handle.SetValue(0.0f, TexType::Anisotropic);
		//	}
		//	if (!mat->HasProperty(TexType::Sheen)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Sheen });
		//		handle.SetValue(0.0f, TexType::Sheen);
		//	}
		//	if (!mat->HasProperty(TexType::SheenTint)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::SheenTint });
		//		handle.SetValue(0.0f, TexType::SheenTint);
		//	}
		//	if (!mat->HasProperty(TexType::Clearcoat)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::Clearcoat });
		//		handle.SetValue(0.0f, TexType::Clearcoat);
		//	}
		//	if (!mat->HasProperty(TexType::ClearcoatGloss)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::ClearcoatGloss });
		//		handle.SetValue(0.0f, TexType::ClearcoatGloss);
		//	}
		//	if (!mat->HasProperty(TexType::SpecTrans)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::SpecTrans });
		//		handle.SetValue(0.0f, TexType::SpecTrans);
		//	}
		//	if (!mat->HasProperty(TexType::ior)) {
		//		mat->AddMaterialProperties({ 1.5f, TexType::ior });
		//		handle.SetValue(1.5f, TexType::ior);
		//	}
		//	if (!mat->HasProperty(TexType::AO)) {
		//		mat->AddMaterialProperties({ 0.0f, TexType::AO });
		//	}

		//	auto& materialComp = entityManager->AddComponent<MaterialComponent>(meshEntity, mat, (int32_t)m_TextureHandles.size());
		//	m_TextureHandles.push_back(handle);

		//	renderDataAll.push_back(renderData);
		//	entityManager->GetComponent<RootComponent>(gameObject).entities.push_back(meshEntity.GetEntityHandle());
		//}

		//if (m_BuildBVH) {
		//	ScopedTimer timer("Build BVH");
		//	BVHi& tlasBvh = m_CurrentLevel->GetTLAS();
		//	BVHComponent& bvhComp = entityManager->AddComponent<BVHComponent>(gameObject);
		//	for (const auto& info : *asset) {
		//		//BVHi* bvhi = new BVHi(info, s_globalSubMeshId++, m_MatMaskEnums[s_globalSubMeshId - 1]); // bad
		//		//BVHi* bvhi = new BVHi(info, s_globalSubMeshId, m_MatMaskEnums[s_globalSubMeshId++]); // bad
		//		BVHi* bvhi = new BVHi(info, s_globalSubMeshId++); 
		//		//BVHi* bvhi = new BVHi(info, s_globalSubMeshId, m_MatMaskEnums[s_globalSubMeshId]); // good
		//		//s_globalSubMeshId += 1;

		//		bvhComp.bvhs.push_back(bvhi);
		//		tlasBvh.AddBLASPrimtive(bvhi);
		//	}
		//	tlasBvh.UpdateTLAS();
		//	PathTracingPipeline* ptpl = static_cast<PathTracingPipeline*>(m_RenderLayer->GetRenderer()->GetPipeline(RenderPipelineType::RPL_PathTracing));
		//	//ptpl->UpdateSSBO(m_CurrentLevel);
		//}

		//UpdatePathTracingTextureHandlesSSBO();
		////asset.reset();
		//UploadRenderDataEventTrigger(renderDataAll);
	}

	// Assume skeletal mesh is a whole mesh
	void LevelLayer::LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset) {
		//auto entityManager = m_CurrentLevel->GetEntityManager();

		//Entity gameObject(entityManager->CreateEntity()); // TODO: give it a proper name
		//entityManager->AddComponent<RootComponent>(gameObject);
		//TransformParam* param = new TransformParam();
		//entityManager->AddComponent<TransformComponent>(gameObject, param);
		//auto& skeletalComponent = entityManager->AddComponent<SkeletalComponent>(gameObject, asset->GetRoot(), m_SkeletalMeshBoneOffset);

		//m_SkeletalMeshBoneOffset += asset->GetBoneCnt();
		//uint32_t entityID = (uint32_t)gameObject.GetEntityHandle();
		//std::vector<std::shared_ptr<RenderData>> renderDataAll;
		//std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCached;
		//renderDataAll.reserve(asset->size());
		//int index = 0;
		//for (const auto& skMeshInfo : *asset) {
		//	std::shared_ptr<VertexArray> vao;
		//	vao.reset(VertexArray::Create());
		//	vao->Init(skMeshInfo);
		//	std::shared_ptr<RenderData> renderData = std::make_shared<RenderData>();
		//	renderData->SetVAOs(vao);
		//	std::shared_ptr<Material> mat = std::make_shared<Material>();
		//	renderData->SetEntityID(entityID);
		//	renderData->GetBoneOffset() = skeletalComponent.offset;
		//	renderData->SetMaterial(mat);
		//	if (skMeshInfo->materialInfo.HasMaterial()) {
		//		for (const auto& [type, path] : skMeshInfo->materialInfo.materials) {
		//			if (!textureCached.contains(path)) {
		//				std::shared_ptr<Texture2D> tex = Texture2D::Create(path);
		//				tex->SetTexType(type);
		//				textureCached[path] = tex;
		//			}
		//			mat->AddMaterialProperties({ textureCached.at(path), type });
		//		}
		//	}
		//	if (!mat->HasProperty(TexType::Albedo)) {
		//		mat->AddMaterialProperties({ glm::vec3(0.95f), TexType::Albedo });
		//	}
		//	renderData->SetTransformParam(param);
		//	renderDataAll.push_back(renderData);
		//}
		//
		//asset.reset();
		//UploadRenderDataEventTrigger(renderDataAll);
	}
} // namespace Aho