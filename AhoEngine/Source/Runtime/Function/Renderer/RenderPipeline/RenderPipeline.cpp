#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto renderPass : m_RenderPasses) {
			delete renderPass;
		}
	}

	void RenderPipeline::Execute() {
		std::shared_ptr<Framebuffer> depthPassFBO{ nullptr };   // Seems like a bad way but can't tell the reason
		for (const auto& renderPass : m_RenderPasses) {
			if (renderPass->GetRenderPassType() != RenderPassType::Debug) {
				auto fbo = renderPass->Execute(m_RenderData, depthPassFBO);
				if (renderPass->GetRenderPassType() == RenderPassType::Depth) {
					depthPassFBO = fbo;
				}
			}
		}
		if (m_DrawDebug && depthPassFBO) {
			for (const auto& renderPass : m_RenderPasses) {
				if (renderPass->GetRenderPassType() == RenderPassType::Debug) {
					renderPass->Execute(m_DebugData, depthPassFBO);
				}
			}
		}
	}

	RenderPass* RenderPipeline::GetRenderPass(size_t index) {
		if (index >= m_RenderPasses.size()) {
			AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
			return nullptr;
		}
		return m_RenderPasses[index];
	}

	void RenderPipeline::SortRenderPasses() {
		std::sort(m_RenderPasses.begin(), m_RenderPasses.end(), [](RenderPass* lhs, RenderPass* rhs) {
			if (lhs->GetRenderPassType() == RenderPassType::Depth) {
				return true;
			}
			return false;
		});
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
	}
} // namespace Aho