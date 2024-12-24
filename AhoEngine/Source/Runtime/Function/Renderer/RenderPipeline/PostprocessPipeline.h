#pragma once
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class DeferredShadingPipeline;

	class PostprocessPipeline : public RenderPipeline {
	public:
		PostprocessPipeline() { Initialize(); }
		virtual void Initialize() override;
		virtual void SetInput(Texture* tex) override;
	private:
		std::unique_ptr<RenderPass> SetupDrawSelectedPass();
		std::unique_ptr<RenderPass> SetupDrawSelectedOutlinePass();
		std::unique_ptr<RenderPass> SetupFXAAPass();

	private:
		std::unique_ptr<RenderPass> m_DrawSelectedPass;
		std::unique_ptr<RenderPass> m_DrawSelectedOutlinePass;
		std::unique_ptr<RenderPass> m_FXAAPass;

	private:
		std::unique_ptr<RenderPass> SetupTestQuadPass();
		std::unique_ptr<RenderPass> m_TestQuadPass;
	};

}