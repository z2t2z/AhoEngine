#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class SkyAtmosphericPipeline : public RenderPipeline {
	public:
		SkyAtmosphericPipeline();
		~SkyAtmosphericPipeline() = default;
		virtual void Initialize() override;
		virtual void Execute() override;
	private:
		std::unique_ptr<RenderPassBase> m_TransmittanceLutPass;
		std::unique_ptr<RenderPassBase> m_MutiScattLutPass;
		std::unique_ptr<RenderPassBase> m_SkyViewLutPass;
	};
}
