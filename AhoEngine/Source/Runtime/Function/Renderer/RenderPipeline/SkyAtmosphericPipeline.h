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
	protected:
		// m_TextureBuffers: [0]: Transmittance LUT, [1]: Multi Scattering LUT, [2]: Sky View LUT
	private:
		std::unique_ptr<RenderPassBase> m_TransmittanceLutPass;
		std::unique_ptr<RenderPassBase> m_MutiScattLutPass;
		std::unique_ptr<RenderPassBase> m_SkyViewLutPass;
	};
}
