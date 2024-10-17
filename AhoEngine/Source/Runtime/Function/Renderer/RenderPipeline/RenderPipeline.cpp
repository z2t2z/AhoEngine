#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto renderPass : m_RenderPasses) {
			delete renderPass;
		}
	}

	void RenderPipeline::Execute() {
		std::shared_ptr<Framebuffer> depthPassFBO = m_DepthPass->Execute(m_RenderData);
		std::shared_ptr<Framebuffer> resultPassFBO = m_ResultPass->Execute(m_RenderData, depthPassFBO);
		m_PickingPass->Execute(m_VirtualData);
		if (m_DrawDebug) {
			m_DebugPass->Execute(m_DebugData, depthPassFBO);
		}
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPass(size_t index) {
		if (index >= m_RenderPasses.size()) {
			AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
			return nullptr;
		}
		return m_RenderPasses[index]->GetRenderTarget();
	}

	void RenderPipeline::SortRenderPasses() {
		//std::sort(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* lhs, RenderPass* rhs) {
		//	if (lhs->GetRenderPassType() == RenderPassType::Depth) {
		//		return true;
		//	}
		//	return false;
		//});
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
	}
} // namespace Aho