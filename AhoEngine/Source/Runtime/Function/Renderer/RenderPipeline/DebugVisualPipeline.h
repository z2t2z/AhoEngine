#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class RenderPass;
	class DebugVisualPipeline : public RenderPipeline {
	public:
		DebugVisualPipeline();
		~DebugVisualPipeline() = default;
		virtual void Initialize() override;
	private:
		std::unique_ptr<RenderPass> SetupLightVisualPass();
		std::unique_ptr<RenderPass> SetupSelectedOutlinePass();
		std::unique_ptr<RenderPass> SetupInfiniteGridPass();
	private:
		std::unique_ptr<RenderPass> m_LightVisualPass;
		std::unique_ptr<RenderPass> m_InfiniteGridPass;
	};
}