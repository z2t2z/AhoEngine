#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "RenderCommand.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Function/Renderer/BufferObject/UBOManager.h"

#include <chrono>
#include <thread>

namespace Aho {
	int RendererGlobalState::g_SelectedEntityID = -1;
	bool RendererGlobalState::g_ShowDebug = false;
	bool RendererGlobalState::g_IsEntityIDValid = false;
	std::shared_ptr<RenderData> RendererGlobalState::g_SelectedData = nullptr;
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	Renderer::Renderer() {
	}

	void Renderer::Initialize() {
		TextureBuffer::Init();
		RenderTask::Init();
		SetupUBOs();
		RenderCommand::SetDepthTest(true);

		m_RP_IBL = new IBLPipeline();

		m_RP_Sky = new RenderSkyPipeline();
		m_RP_DeferredShading = new DeferredShadingPipeline();
		m_RP_DeferredShading->SetSunDir(m_RP_Sky->GetSunDir());

		m_RP_Postprocess = new PostprocessPipeline();
		m_RP_Dbg = new DebugVisualPipeline();


		// --- New System ---
		m_RP_SkyAtmospheric = new SkyAtmosphericPipeline();
		m_RP_Derferred = new DeferredShading();
		m_RP_PathTracing = new PathTracingPipeline();
		m_RP_IBLPipeline = new _IBLPipeline();

		auto shadingResFBO = m_RP_DeferredShading->GetRenderPassTarget(RenderPassType::Shading);
		m_RP_Dbg->SetRenderTarget(RenderPassType::DebugVisual, shadingResFBO);

		// Register buffers from ibl
		auto rp_deferred = m_RP_DeferredShading->GetRenderPass(RenderPassType::Shading);
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::PrecomputeIrradiance)->GetTextureBuffer(TexType::Irradiance), TexType::Irradiance });
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::GenLUT)->GetTextureBuffer(TexType::BRDFLUT), TexType::BRDFLUT });
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::Prefilter)->GetTextureBuffer(TexType::Prefiltering), TexType::Prefiltering });

		// Register buffers from skyview
		rp_deferred->RegisterTextureBuffer({ m_RP_Sky->GetRenderResult(), TexType::SkyViewLUT });

		m_RP_Postprocess->SetInput(m_RP_DeferredShading->GetRenderResult());

		//m_RP_Dbg->SetInput(m_RP_DeferredShading->GetRenderResult());
	}


	using Clock = std::chrono::steady_clock;
	constexpr double targetFPS = 60.0;
	constexpr auto frameSecs = std::chrono::duration<double>(1.0 / targetFPS);
	constexpr auto frameDur =
		std::chrono::duration_cast<Clock::duration>(frameSecs);

	static auto s_previous = Clock::now();
	void Renderer::Render(float deltaTime) {
		auto frameStart = Clock::now();
		std::chrono::duration<double> dt = frameStart - s_previous;
		m_FrameTime = dt.count();
		s_previous = frameStart;

		// --- New System
		m_RP_Derferred->Execute();
		m_RP_SkyAtmospheric->Execute();
		m_RP_IBLPipeline->Execute();

		if (m_CurrentRenderMode == RenderMode::DefaultLit) {
			m_RP_Sky->Execute();
			m_RP_DeferredShading->Execute();
			//m_RP_Postprocess->Execute();
			m_RP_Dbg->Execute();
		}
		else if (m_CurrentRenderMode == RenderMode::PathTracing) {
			if (m_CameraDirty) {
				m_RP_PathTracing->ClearAccumulateData();
				m_CameraDirty = false;
			}
			m_RP_PathTracing->Execute();
		}

		auto nextFrameTime = frameStart + frameDur;
		//std::this_thread::sleep_until(nextFrameTime);
	}

	RenderPipeline* Renderer::GetPipeline(RenderPipelineType type) {
		switch (type) {
		case RenderPipelineType::RPL_DeferredShading:
			return m_RP_DeferredShading;
		case RenderPipelineType::RPL_RenderSky:
			return m_RP_Sky;
		case RenderPipelineType::RPL_PostProcess:
			return m_RP_Postprocess;
		case RenderPipelineType::RPL_PathTracing:
			return m_RP_PathTracing;
		case RenderPipelineType::RPL_IBL:
			return m_RP_IBL;
		case RenderPipelineType::RPL_DebugVisual:
			return m_RP_Dbg;
		default:
			AHO_CORE_ASSERT(false);
		}
		AHO_CORE_ASSERT(false);
	}

	void Renderer::SetCameraDirty() {
		m_CameraDirty = true;
	}

	bool Renderer::OnViewportResize(uint32_t width, uint32_t height) {
		if (m_CurrentRenderMode == RenderMode::DefaultLit) {
			return m_RP_Derferred->Resize(width, height);


			if (m_RP_Postprocess->ResizeRenderTarget(width, height)) {
				m_RP_DeferredShading->ResizeRenderTarget(width, height);
				return true;
			}
			return false;
		}
		else {
			return m_RP_PathTracing->Resize(width, height);

			return m_RP_PathTracing->ResizeRenderTarget(width, height);
		}
	}

	// TODO: Fix fxaa: it is appiled to all pixels
	uint32_t Renderer::GetRenderResultTextureID() {
		if (m_CurrentRenderMode == RenderMode::DefaultLit) {
			//return m_RP_Derferred->GetRenderResult()->GetTextureID();
			return m_RP_Derferred->GetRenderResultTextureID();
		}
		else if (m_CurrentRenderMode == RenderMode::PathTracing) {
			//return m_RP_PathTracing->GetRenderResult()->GetTextureID();
			return m_RP_PathTracing->GetRenderResultTextureID();
		}
		return m_RP_DeferredShading->GetRenderResult()->GetTextureID();
		return m_RP_Postprocess->GetRenderResult()->GetTextureID();
	}

	void Renderer::AddRenderData(const std::shared_ptr<RenderData>& data) {
		RenderTask::m_SceneData.push_back(data);
	}

	void Renderer::AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) {
		for (const auto& d : data) AddRenderData(d);
	}

	void Renderer::SetupUBOs() {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<LightUBO>(1);
		UBOManager::RegisterUBO<RandomKernelUBO>(2); RandomKernelUBO rndUBO; UBOManager::UpdateUBOData(2, rndUBO);
		UBOManager::RegisterUBO<AnimationUBO>(3);
		UBOManager::RegisterUBO<SkeletonUBO>(4);
	}

}