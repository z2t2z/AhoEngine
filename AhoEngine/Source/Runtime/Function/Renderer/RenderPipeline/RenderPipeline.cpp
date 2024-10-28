#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto renderPass : m_RenderPasses) {
			delete renderPass;
		}
	}

	void RenderPipeline::Execute() {
		m_ShadowMapPass->Execute(m_RenderData);
		m_GBufferPass->Execute(m_RenderData);
		m_DebugPass->Execute(m_DebugData);
		m_HiZPass->Execute(m_ScreenQuad);
		m_SSAOPass->Execute(m_ScreenQuad);
		m_BlurPass->Execute(m_ScreenQuad);
		m_SSRvsPass->Execute(m_ScreenQuad);
		m_ShadingPass->Execute(m_ScreenQuad);
		m_PostProcessingPass->Execute(m_ScreenQuad);
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPassTarget(RenderPassType type) {
		switch (type) {
			case RenderPassType::Debug:
				return m_DebugPass->GetRenderTarget();
			case RenderPassType::Depth:
				return m_ShadowMapPass->GetRenderTarget();
			case RenderPassType::SSAO:
				return m_SSAOPass->GetRenderTarget();
			case RenderPassType::SSAOGeo:
				return m_GBufferPass->GetRenderTarget();
			case RenderPassType::SSAOLighting:
				return m_SSAOLightingPass->GetRenderTarget();
			case RenderPassType::Shading:
				return m_ShadingPass->GetRenderTarget();
			case RenderPassType::Pick:
				return m_PickingPass->GetRenderTarget();
			case RenderPassType::Blur:
				return m_BlurPass->GetRenderTarget();
			case RenderPassType::SSRvs:
				return m_SSRvsPass->GetRenderTarget();
			case RenderPassType::HiZ:
				return m_HiZPass->GetRenderTarget();
			case RenderPassType::DrawLine:
				return m_DrawLinePass->GetRenderTarget();
			case RenderPassType::PostProcessing:
				return m_PostProcessingPass->GetRenderTarget();
		}
		AHO_CORE_ASSERT(0);
	}

	// TODO: devise a better way
	void RenderPipeline::SortRenderPasses() {
		auto it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Shading;
			});
		if (it != m_RenderPasses.end()) {
			m_ShadingPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Debug;
			});
		if (it != m_RenderPasses.end()) {
			m_DebugPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Depth;
			});
		if (it != m_RenderPasses.end()) {
			m_ShadowMapPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Pick;
			});
		if (it != m_RenderPasses.end()) {
			m_PickingPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::SSAOGeo;
			});
		if (it != m_RenderPasses.end()) {
			m_GBufferPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::SSAO;
			});
		if (it != m_RenderPasses.end()) {
			m_SSAOPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::SSAOLighting;
			});
		if (it != m_RenderPasses.end()) {
			m_SSAOLightingPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Blur;
			});
		if (it != m_RenderPasses.end()) {
			m_BlurPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::SSRvs;
			});
		if (it != m_RenderPasses.end()) {
			m_SSRvsPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::HiZ;
			});
		if (it != m_RenderPasses.end()) {
			m_HiZPass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::DrawLine;
			});
		if (it != m_RenderPasses.end()) {
			m_DrawLinePass = *it;
		}
		it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::PostProcessing;
			});
		if (it != m_RenderPasses.end()) {
			m_PostProcessingPass = *it;
		}
	}
} // namespace Aho