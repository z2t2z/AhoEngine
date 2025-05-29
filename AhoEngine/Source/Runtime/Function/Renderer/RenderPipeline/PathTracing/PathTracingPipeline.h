#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class PathTracingPipeline : public RenderPipeline {
	public:
		PathTracingPipeline();
		~PathTracingPipeline() = default;
		virtual void Initialize() override;
		virtual void Execute() override;
		virtual bool Resize(uint32_t width, uint32_t height) const override;
		void SetEnvMap(Texture* texture) {}
	private:
		std::unique_ptr<RenderPassBase> m_AccumulatePass;
		std::unique_ptr<RenderPassBase> m_PresentPass;
	private:
		uint32_t m_Frame{ 1 };
	};
}