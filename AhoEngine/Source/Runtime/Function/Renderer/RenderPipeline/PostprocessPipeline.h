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
		std::shared_ptr<_Texture> m_ObjectID;
		std::shared_ptr<_Texture> m_PickingDepth;
		std::shared_ptr<_Texture> m_Grid;
	private:
		std::unique_ptr<RenderPassBase> m_PickingPass;
		std::unique_ptr<RenderPassBase> m_SingleDepthPass;
		std::unique_ptr<RenderPassBase> m_OutlinePass;
		std::unique_ptr<RenderPassBase> m_DrawGridPass;

	// --- Delete these ---
	private:
		std::unique_ptr<RenderPass> SetupFXAAPass();
	private:
		std::unique_ptr<RenderPass> m_FXAAPass;
	};

}