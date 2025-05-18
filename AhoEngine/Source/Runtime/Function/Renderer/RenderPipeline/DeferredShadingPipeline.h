#pragma once
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

#include <memory>
#include <glm/common.hpp>

namespace Aho {
	class RenderPass;
	struct IBLLuts;

	class DeferredShadingPipeline : public RenderPipeline {
	public:
		DeferredShadingPipeline() { Initialize(); }
		virtual void Initialize() override;
		void SetSunDir(const glm::vec3& dir) { m_SunDir = dir; }
		void SetEnvLightState(bool state);
		void SetIBLLuts(const IBLLuts& luts);
	private:
		std::unique_ptr<RenderPass> SetupShadowMapPass(); // TODO: multiple shadow map passes
		std::unique_ptr<RenderPass> SetupGBufferPass();
		std::unique_ptr<RenderPass> SetupShadingPass();
		std::unique_ptr<RenderPass> SetupSSAOPass();
		std::unique_ptr<RenderPass> SetupBlurRPass(); // bluring r channel only, for ssao use

	private:
		bool m_EnvLightState{ false };
		glm::vec3 m_SunDir;
	private:
		std::unique_ptr<RenderPass> m_ShadowMapPass;
		std::unique_ptr<RenderPass> m_GBufferPass;
		std::unique_ptr<RenderPass> m_SSAOPass;
		std::unique_ptr<RenderPass> m_BlurRPass;
		std::unique_ptr<RenderPass> m_ShadingPass;

	};

}