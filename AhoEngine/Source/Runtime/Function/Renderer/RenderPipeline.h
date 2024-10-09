#pragma once

#include "RenderPass.h"
#include <vector>

namespace Aho {
	class RenderPipeline {
	public:
		~RenderPipeline() = default;
		virtual void Execute() const {
			for (const auto& renderPass : m_RenderPasses) {
				renderPass->Execute();
			}
		}
		RenderPass* GetRenderPass(size_t index) {
			if (index >= m_RenderPasses.size()) {
				AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
				return nullptr;
			}
			return m_RenderPasses[index];
		}
		virtual void Initialize() = 0;
		virtual void AddRenderPass(RenderPass* rp) { m_RenderPasses.push_back(rp); }
	protected:
		std::vector<RenderPass*> m_RenderPasses; // Must be ordered!
	};


	class RenderPipelineDefault : public RenderPipeline {
	public:
		RenderPipelineDefault() {
			Initialize();
		}
		virtual void Initialize() override {
			AHO_CORE_INFO("RenderPipelineDefault initialized");
		}
	};
} // namespace Aho
