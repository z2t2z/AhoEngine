#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto renderPass : m_RenderPasses) {
			delete renderPass;
		}
	}

	void RenderPipeline::Execute() {
		m_DepthPass->Execute(m_RenderData);
		m_ResultPass->Execute(m_RenderData);
		m_PickingPass->Execute(m_VirtualData);
		if (m_DrawDebug) {
			m_DebugPass->Execute(m_DebugData);
		}
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPass(size_t index) {
		if (index >= m_RenderPasses.size()) {
			AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
			return nullptr;
		}
		return m_RenderPasses[index]->GetRenderTarget();
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
			m_SSAO = *it;
		}
	}
} // namespace Aho