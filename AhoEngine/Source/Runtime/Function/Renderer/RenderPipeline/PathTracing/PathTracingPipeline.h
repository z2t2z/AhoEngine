#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {

	class Level;

	class PathTracingPipeline : public RenderPipeline {
	public:
		PathTracingPipeline();
		virtual void Initialize() override;
		void UpdateSSBO(const std::shared_ptr<Level>& currlevel);

	private:
		std::unique_ptr<RenderPass> SetupGBufferPass();
		std::unique_ptr<RenderPass> SetupShadingPass();

	private:
		std::unique_ptr<RenderPass> m_ShadingPass;
		std::unique_ptr<RenderPass> m_GbufferPass;


	private:
		uint32_t m_Frame;
	};

}