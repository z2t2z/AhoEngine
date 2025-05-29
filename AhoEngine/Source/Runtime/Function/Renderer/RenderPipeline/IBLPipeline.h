#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class RenderPassBase;
	class _IBLPipeline : public RenderPipeline {
	public:
		_IBLPipeline() { Initialize(); }
		~_IBLPipeline() = default;
		virtual void Initialize() override;
		virtual void Execute() override;
	private:
		std::unique_ptr<RenderPassBase> m_RP_GenCubemapFromSphericalMap;
		std::unique_ptr<RenderPassBase> m_RP_PrecomputeIrradiance;		// Diffuse 
		std::unique_ptr<RenderPassBase> m_RP_Prefiltering;				// Specular
		std::unique_ptr<RenderPassBase> m_RP_BRDFLUT;
	};
}