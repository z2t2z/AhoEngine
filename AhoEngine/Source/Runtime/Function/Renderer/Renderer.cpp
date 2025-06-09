#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "RenderCommand.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Function/Renderer/BufferObject/UBOManager.h"

#include <chrono>
#include <thread>

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	Renderer::Renderer() {
	}

	void Renderer::Initialize() {
		TextureBuffer::Init();
		RenderTask::Init();
		SetupUBOs();
		RenderCommand::SetDepthTest(true);

		// --- New System ---
		m_RP_SkyAtmospheric = new SkyAtmosphericPipeline();
		m_RP_Derferred = new DeferredShading();
		m_RP_PathTracing = new PathTracingPipeline();
		m_RP_IBLPipeline = new _IBLPipeline();

		SetRenderMode(RenderMode::DefaultLit);
	}

	void Renderer::Render(float deltaTime) {
		UpdateUBOs();
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
		if (m_CurrentRenderMode == RenderMode::DefaultLit)
			return m_RP_Derferred->Resize(width, height);
		else
			return m_RP_PathTracing->Resize(width, height);
	}

	void Renderer::SetupUBOs() const {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<GPU_DirectionalLight>(1);
		UBOManager::RegisterUBO<AnimationUBO>(2);
	}

	void Renderer::UpdateUBOs() const {
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		ecs->GetView<LightComponent, LightDirtyTagComponent>().each(
			[&ecs](Entity entity, const LightComponent& lcomp, const LightDirtyTagComponent& dirtyTag) {
				const std::shared_ptr<Light>& light = lcomp.light;
				if (light->GetType() == LightType::Directional) {
					GPU_DirectionalLight gpuLight;
					auto dirLight = std::static_pointer_cast<DirectionalLight>(light);
					FillDirectionalLightStruct(gpuLight, dirLight);
					UBOManager::UpdateUBOData<GPU_DirectionalLight>(5, gpuLight, lcomp.index);
					ecs->RemoveComponent<LightDirtyTagComponent>(entity);
				}
			}
		);
	}
}