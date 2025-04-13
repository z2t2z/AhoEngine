#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class DebugVisualPipeline : public RenderPipeline {
	public:
		DebugVisualPipeline();
		~DebugVisualPipeline() = default;
		virtual void Initialize() override;
		//virtual void SetRenderTarget(RenderPassType type, const std::shared_ptr<Framebuffer>& fbo) override;
	private:
		std::unique_ptr<RenderPass> SetupLightVisualPass();
	private:
		std::unique_ptr<RenderPass> m_LightVisualPass;
	};
}