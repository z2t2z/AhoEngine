#pragma once
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"

#include <memory>
#include <glm/glm.hpp>

namespace Aho {
	class _Texture;
	class DeferredShading : public RenderPipeline {
	public:
		DeferredShading() { Initialize(); }
		~DeferredShading() = default;
		virtual void Initialize() override;
		virtual void Execute() override;
		virtual bool Resize(uint32_t width, uint32_t height) const override;
	private:
		std::unique_ptr<RenderPassBase> m_ShadowMapPass;
		std::unique_ptr<RenderPassBase> m_GBufferPass;
		std::unique_ptr<RenderPassBase> m_SSAOPass;
		std::unique_ptr<RenderPassBase> m_BlurRPass;
		std::unique_ptr<RenderPassBase> m_ShadingPass;
	private:
		std::unique_ptr<RenderPassBase> m_DepthPyramidPass;
		std::unique_ptr<RenderPassBase> m_SSRPass;
	private:
		// G_Buffers
		std::shared_ptr<_Texture> m_Position;
		std::shared_ptr<_Texture> m_Depth;
		std::shared_ptr<_Texture> m_Normal;
		std::shared_ptr<_Texture> m_PBRPack;
		std::shared_ptr<_Texture> m_Albedo;


		std::shared_ptr<_Texture> m_DepthPyramid;

		_Texture* m_SceneDepth{ nullptr };
		_Texture* m_SSRTex{ nullptr };
		_Texture* m_SceneDepthPyramid{ nullptr };
	};
}
