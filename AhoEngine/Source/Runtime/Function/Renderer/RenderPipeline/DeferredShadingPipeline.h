#pragma once
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class DeferredShadingPipeline : public RenderPipeline {
	public:
		DeferredShadingPipeline() { Initialize(); }
		virtual void Initialize() override;

	private:
		std::unique_ptr<RenderPass> SetupShadowMapPass(); // TODO: multiple shadow map passes
		std::unique_ptr<RenderPass> SetupGBufferPass();
		std::unique_ptr<RenderPass> SetupShadingPass();
		std::unique_ptr<RenderPass> SetupSSAOPass();
		std::unique_ptr<RenderPass> SetupBlurRPass(); // bluring r channel only, for ssao use

	private:
		std::unique_ptr<RenderPass> m_ShadowMapPass;
		std::unique_ptr<RenderPass> m_GBufferPass;
		std::unique_ptr<RenderPass> m_SSAOPass;
		std::unique_ptr<RenderPass> m_BlurRPass;
		std::unique_ptr<RenderPass> m_ShadingPass;

	};
}