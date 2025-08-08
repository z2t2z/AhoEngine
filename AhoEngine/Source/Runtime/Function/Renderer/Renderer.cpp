#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "RenderCommand.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/BufferObject/UBOManager.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "RenderPipeline/DDGI/DDGIPipeline.h"
#include "RenderPipeline/PostprocessPipeline.h"
#include "RenderPipeline/DeferredShadingPipeline.h"
#include "RenderPipeline/PathTracing/PathTracingPipeline.h"
#include "RenderPipeline/SkyAtmosphericPipeline.h"
#include "RenderPipeline/DeferredPipeline.h"
#include "RenderPipeline/IBLPipeline.h"
#include "RenderPipeline/DebugVisualPipeline.h"

#include <glad/glad.h>
#include <chrono>
#include <thread>

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	Renderer::Renderer() {
	}

	Renderer::~Renderer() {
		delete m_RP_PathTracing;
		delete m_RP_Derferred;
		delete m_RP_SkyAtmospheric;
		delete m_RP_IBLPipeline;
		delete m_RP_Postprocess;
	}

	void Renderer::SetRenderMode(RenderMode mode) {
		m_CurrentRenderMode = mode;
		if (mode == RenderMode::PathTracing) {
			m_ActivePipelines = { m_RP_PathTracing };
		} else {
			m_ActivePipelines = { m_RP_IBLPipeline, m_RP_SkyAtmospheric, m_RP_Derferred, m_DDGIPipeline }; //this is wrong!!
		}
		m_ActivePipelines.push_back(m_RP_Postprocess);
		SetViewportDisplayTextureBuffer(m_RP_Postprocess->GetRenderResultTextureBuffer());
	}

	void Renderer::Initialize() {
		TextureBuffer::Init();
		RenderTask::Init();
		SetupUBOs();
		RenderCommand::SetDepthTest(true);

		// --- New System ---
		m_RP_SkyAtmospheric = new SkyAtmosphericPipeline();
		m_RP_Derferred		= new DeferredShading();
		m_RP_PathTracing	= new PathTracingPipeline();
		m_RP_IBLPipeline	= new _IBLPipeline();
		m_RP_Postprocess	= new PostprocessPipeline();
		m_DDGIPipeline		= new DDGIPipeline();

		SetRenderMode(RenderMode::DefaultLit);
	}

	void Renderer::Render(float deltaTime) {
		UpdateUBOs();
		UpdateSSBOs();
		for (const auto& pipeline : m_ActivePipelines) {
			pipeline->Execute();
		}
	}

	RenderPipeline* Renderer::GetPipeline(RenderPipelineType type) {
		switch (type) {
			case RenderPipelineType::RPL_DeferredShading:
				return m_RP_Derferred;
			case RenderPipelineType::RPL_RenderSky:
				return m_RP_SkyAtmospheric;
			case RenderPipelineType::RPL_PostProcess:
				return m_RP_Postprocess;
			case RenderPipelineType::RPL_PathTracing:
				return m_RP_PathTracing;
			case RenderPipelineType::RPL_IBL:
				return m_RP_IBLPipeline;
			case RenderPipelineType::RPL_DebugVisual:
				return m_RP_Dbg;
			default:
				AHO_CORE_ASSERT(false);
		}
		AHO_CORE_ASSERT(false);
	}

	bool Renderer::OnViewportResize(uint32_t width, uint32_t height) {
		bool resized = false;
		for (const auto& pipeline : m_ActivePipelines) {
			resized |= pipeline->Resize(width, height);
		}
		return resized;
	}

	void Renderer::RegisterRenderPassBase(RenderPassBase* rp) {
		m_AllRenderPasses.push_back(rp); 
		m_RenderPasses[rp->GetPassName()] = rp;
	}

	void Renderer::SetupUBOs() const {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<GPU_DirectionalLight>(1);
		UBOManager::RegisterUBO<AnimationUBO>(2);
	}

	// Camera, light data
	void Renderer::UpdateUBOs() const {
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		ecs->GetView<LightComponent, LightDirtyTagComponent>().each(
			[&ecs](Entity entity, const LightComponent& lcomp, const LightDirtyTagComponent& dirtyTag) {
				const std::shared_ptr<Light>& light = lcomp.light;
				if (light->GetType() == LightType::Directional) {
					GPU_DirectionalLight gpuLight;
					auto dirLight = std::static_pointer_cast<DirectionalLight>(light);
					FillDirectionalLightStruct(gpuLight, dirLight);
					UBOManager::UpdateUBOData<GPU_DirectionalLight>(1, gpuLight, lcomp.index);
					ecs->RemoveComponent<LightDirtyTagComponent>(entity);
				}
			}
		);

		ecs->GetView<EditorCamaraComponent>().each(
			[&ecs](Entity entity, const EditorCamaraComponent& camComp) {
				const auto& cam = camComp.camera;
				CameraUBO camUBO;
				camUBO.u_View = cam->GetView();
				camUBO.u_Projection = cam->GetProjection();
				camUBO.u_ProjectionInv = cam->GetProjectionInv();
				camUBO.u_ViewInv = cam->GetViewInv();
				camUBO.u_ProjView = cam->GetProjection() * cam->GetView();
				camUBO.u_ViewPosition = glm::vec4(cam->GetPosition(), 1.0f);
				UBOManager::UpdateUBOData<CameraUBO>(0, camUBO);
			}
		);
	}

	// Basically bvh data
	// Needs big refactor
	void Renderer::UpdateSSBOs() const {
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		bool dirty = false;
		static bool BVHDirty = true;
		auto view = ecs->GetView<_BVHComponent, _TransformComponent, _MaterialComponent>();
		std::vector<BVHi*> tasks; //tasks.reserve(view.size()); // ?
		view.each(
			[&](auto entity, _BVHComponent& bvh, _TransformComponent& transform, _MaterialComponent& material) {
				int meshID = bvh.bvh->GetMeshId();
				if (transform.Dirty) {
					transform.Dirty = false; // TODO: Should not be here, but in inspector panel
					dirty = true;
					bvh.bvh->ApplyTransform(transform.GetTransform());
					tasks.emplace_back(bvh.bvh.get());
					BVHDirty = true;
				}
				if (material.Dirty) {
					material.Dirty = false; // TODO: Should not be here, but in inspector panel
					dirty = true;
					material.mat.UpdateMaterialDescriptor();
					SSBOManager::UpdateSSBOData<MaterialDescriptor>(5, { material.mat.GetMatDescriptor() }, meshID);
				}
			}
		);
		{
			const auto& executor = g_RuntimeGlobalCtx.m_ParallelExecutor;
			size_t siz = tasks.size();
			g_RuntimeGlobalCtx.m_ParallelExecutor->ParallelFor(siz,
				[tasks](int64_t i) {
					tasks[i]->Rebuild();
				}, 1
			);
		}
		// No need to update every blas everytime but for now it's fine
		if (BVHDirty) {
			dirty = true;
			BVHDirty = false;
			auto sceneView = ecs->GetView<SceneBVHComponent>();
			AHO_CORE_ASSERT(sceneView.size() <= 1);
			sceneView.each(
				[&](auto entity, SceneBVHComponent& sceneBvh) {
					sceneBvh.bvh->UpdateTLAS();
					const auto& tlas = sceneBvh.bvh;
					SSBOManager::UpdateSSBOData<BVHNodei>(0, tlas->GetNodesArr());
					SSBOManager::UpdateSSBOData<PrimitiveDesc>(1, tlas->GetPrimsArr());
					SSBOManager::UpdateSSBOData<OffsetInfo>(4, tlas->GetOffsetMap());

					size_t nodesOffset = 0;
					size_t primsOffset = 0;
					const std::vector<OffsetInfo>& info = tlas->GetOffsetMap();
					for (size_t i = 0; i < tlas->GetPrimsCount(); i++) {
						const BVHi* blas = tlas->GetBLAS(i);
						AHO_CORE_ASSERT(nodesOffset == info[i].nodeOffset);
						AHO_CORE_ASSERT(primsOffset == info[i].primOffset);
						SSBOManager::UpdateSSBOData<BVHNodei>(2, blas->GetNodesArr(), nodesOffset);
						SSBOManager::UpdateSSBOData<PrimitiveDesc>(3, blas->GetPrimsArr(), primsOffset);
						nodesOffset += blas->GetNodeCount();
						primsOffset += blas->GetPrimsCount();
					}
				}
			);
		}

		if (dirty) {

		}
	}
}