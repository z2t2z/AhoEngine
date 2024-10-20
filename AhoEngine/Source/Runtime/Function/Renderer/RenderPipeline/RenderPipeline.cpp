#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto renderPass : m_RenderPasses) {
			delete renderPass;
		}
		for (auto ubo : m_RenderUBOs) {
			delete ubo;
		}
	}

	void RenderPipeline::Execute() {
		m_DepthPass->Execute(m_RenderData, m_RenderUBOs[0]);
		m_SSAOGeoPass->Execute(m_RenderData, m_RenderUBOs[0]);
		m_SSAOPass->Execute(m_ScreenQuad, m_RenderUBOs[2]);
		m_BlurPass->Execute(m_ScreenQuad);
		//m_SSAOLightingPass->Execute(m_ScreenQuad, m_RenderUBOs[1]);
		m_ResultPass->Execute(m_RenderData, m_RenderUBOs[1]);
		m_PickingPass->Execute(m_VirtualData, m_RenderUBOs[0]);
		if (m_DrawDebug) {
			m_DebugPass->Execute(m_ScreenQuad);
		}
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPass(size_t index) {
		if (index >= m_RenderPasses.size()) {
			AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
			return nullptr;
		}
		return m_RenderPasses[index]->GetRenderTarget();
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPassTarget(RenderPassType type) {
		switch (type) {
			case RenderPassType::Debug:
				return m_DebugPass->GetRenderTarget();
			case RenderPassType::Depth:
				return m_DepthPass->GetRenderTarget();
			case RenderPassType::SSAO:
				return m_SSAOPass->GetRenderTarget();
			case RenderPassType::SSAOGeo:
				return m_SSAOGeoPass->GetRenderTarget();
			case RenderPassType::SSAOLighting:
				return m_SSAOLightingPass->GetRenderTarget();
			case RenderPassType::Final:
				return m_ResultPass->GetRenderTarget();
			case RenderPassType::Pick:
				return m_PickingPass->GetRenderTarget();
			case RenderPassType::Blur:
				return m_BlurPass->GetRenderTarget();
		}
		AHO_CORE_ASSERT(true);
	}

	// TODO
	void RenderPipeline::SortRenderPasses() {
		auto it = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* targetPass) {
			return targetPass->GetRenderPassType() == RenderPassType::Final;
		});
		if (it != m_RenderPasses.end()) {
			m_ResultPass = *it;
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
			m_DepthPass = *it;
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
			m_SSAOGeoPass = *it;
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
	}
} // namespace Aho