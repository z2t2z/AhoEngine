#pragma once
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class _Texture;
	class RenderPassBase;
	class PostprocessPipeline : public RenderPipeline {
	public:
		PostprocessPipeline() { Initialize(); }
		virtual void Initialize() override;

		virtual void Execute() override;
		virtual bool Resize(uint32_t width, uint32_t height) const override;
	private:
		std::shared_ptr<_Texture> m_SelectedDepth;
		std::shared_ptr<_Texture> m_Outlined;
	private:
		std::unique_ptr<RenderPassBase> m_SingleDepthPass;
		std::unique_ptr<RenderPassBase> m_OutlinePass;


	// --- Delete these ---
	private:
		std::unique_ptr<RenderPass> SetupDrawSelectedOutlinePass();
		std::unique_ptr<RenderPass> SetupFXAAPass();
	private:
		std::unique_ptr<RenderPass> m_DrawSelectedOutlinePass;
		std::unique_ptr<RenderPass> m_FXAAPass;
	};

}