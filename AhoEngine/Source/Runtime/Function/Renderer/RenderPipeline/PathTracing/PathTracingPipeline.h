#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {

	class Level;

	class PathTracingPipeline : public RenderPipeline {
	public:
		PathTracingPipeline();
		virtual void Initialize() override;
		void UpdateSSBO(const std::shared_ptr<Level>& currlevel);
		void ClearAccumulateData();

		virtual bool ResizeRenderTarget(uint32_t width, uint32_t height) override;

	private:
		std::unique_ptr<RenderPass> SetupGBufferPass();
		std::unique_ptr<RenderPass> SetupAccumulatePass();

	private:
		std::unique_ptr<RenderPass> m_AccumulatePass;
		std::unique_ptr<RenderPass> m_PresentPass;
		std::unique_ptr<RenderPass> m_GbufferPass;

	private:
		uint32_t m_Frame;
	};

}