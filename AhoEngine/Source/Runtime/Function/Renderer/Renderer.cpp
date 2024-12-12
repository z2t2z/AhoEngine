#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArrayr.h"
#include "RenderCommand.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Function/Renderer/UBOManager.h"

namespace Aho {
	int RendererGlobalState::g_SelectedEntityID = -1;
	bool RendererGlobalState::g_ShowDebug = false;
	bool RendererGlobalState::g_IsEntityIDValid = false;
	std::shared_ptr<RenderData> RendererGlobalState::g_SelectedData = nullptr;

	Renderer::Renderer() {
		TextureBuffer::Init();
		RenderTask::Init();
		SetupUBOs();
		RenderCommand::SetDepthTest(true);

		m_RP_IBL				= new IBLPipeline();
		auto currentPath = std::filesystem::current_path();
		Texture* hdr = new OpenGLTexture2D((currentPath / "Asset" / "HDR" / "rogland_clear_night_4k.hdr").string()/*, true*/);
		hdr->SetTexType(TexType::HDR);
		m_RP_IBL->AddSphericalMap(hdr);

		m_RP_Sky				= new RenderSkyPipeline();
		m_RP_DeferredShading	= new DeferredShadingPipeline();
		m_PathTraciingPipeline	= new PathTracerPipeline();
		m_RP_Postprocess		= new PostprocessPipeline();
		
		// Register buffers from ibl
		auto rp_deferred = m_RP_DeferredShading->GetRenderPass(RenderPassType::Shading);
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::PrecomputeIrradiance)->GetTextureBuffer(TexType::Irradiance), TexType::Irradiance });
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::GenLUT)->GetTextureBuffer(TexType::BRDFLUT), TexType::BRDFLUT });
		rp_deferred->RegisterTextureBuffer({ m_RP_IBL->GetRenderPass(RenderPassType::Prefilter)->GetTextureBuffer(TexType::Prefiltering), TexType::Prefiltering });
		
		// Register buffers from skyview
		rp_deferred->RegisterTextureBuffer({ m_RP_Sky->GetRenderResult(), TexType::SkyViewLUT });

		m_RP_Postprocess->SetInput(m_RP_DeferredShading->GetRenderResult());
	}

	void Renderer::Render() {
		m_RP_Sky->Execute();
		m_RP_DeferredShading->Execute();
		m_RP_Postprocess->Execute();
	}

	bool Renderer::OnViewportResize(uint32_t width, uint32_t height) {
		if (m_RP_Postprocess->ResizeRenderTarget(width, height)) {
			m_RP_DeferredShading->ResizeRenderTarget(width, height);
			return true;
		}
		return false;
	}

	void Renderer::SetupUBOs() {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<LightUBO>(1);
		UBOManager::RegisterUBO<RandomKernelUBO>(2); RandomKernelUBO rndUBO; UBOManager::UpdateUBOData(2, rndUBO);
		UBOManager::RegisterUBO<AnimationUBO>(3);
		UBOManager::RegisterUBO<SkeletonUBO>(4);
	}

}