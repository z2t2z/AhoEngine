#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	// BRDF lut is the same for all ibl sources, still have it here for consistency
	struct IBLLuts {
		IBLLuts() = default;
		IBLLuts(Texture* cm, Texture* brdf, Texture* prefilter, Texture* irr)
			: cubemap(cm), brdfLut(brdf), prefilteringLut(prefilter), irradianceLut(irr) {}
		Texture* cubemap;
		Texture* brdfLut;
		Texture* prefilteringLut;
		Texture* irradianceLut;
	};

	class IBLPipeline : public RenderPipeline {
	public:
		IBLPipeline() { Initialize(); }
		~IBLPipeline() = default;
		virtual void Initialize() override;

		const std::unordered_map<Texture*, IBLLuts>& GetIBLLutsMap() { return m_IBLLutsMap; }
		
		void AddSphericalMap(Texture* sphericalMap) {
			AHO_CORE_ASSERT(sphericalMap != nullptr);
			if (m_IBLLutsMap.contains(sphericalMap)) {
				AHO_CORE_WARN("HDR Map at path:{} already exists", sphericalMap->GetPath());
				return;
			}

			m_RP_GenCubemapFromSphericalMap->RegisterTextureBuffer({ sphericalMap, TexType::HDR });
			m_RP_Prefiltering->RegisterTextureBuffer({ m_RP_GenCubemapFromSphericalMap->GetTextureBuffer(TexType::CubeMap), TexType::CubeMap });
			m_RP_PrecomputeIrradiance->RegisterTextureBuffer({ m_RP_GenCubemapFromSphericalMap->GetTextureBuffer(TexType::CubeMap), TexType::CubeMap });

			IBLLuts luts(m_RP_GenCubemapFromSphericalMap->GetTextureBuffer(TexType::CubeMap),
						m_RP_GenLUT->GetTextureBuffer(TexType::BRDFLUT),
						m_RP_Prefiltering->GetTextureBuffer(TexType::Prefiltering),
						m_RP_PrecomputeIrradiance->GetTextureBuffer(TexType::Irradiance));

			m_IBLLutsMap[sphericalMap] = luts;
			Execute();
			m_CurrSource = sphericalMap;
		}
		IBLLuts GetCurrIBLLuts() const { return m_IBLLutsMap.at(m_CurrSource); }
		void AddCubeMap(Texture* cubeMap) {
			// TODO
			AHO_CORE_ERROR("Not supported yet!");
		}

	private:
		std::unique_ptr<RenderPass> m_RP_GenCubemapFromSphericalMap;
		std::unique_ptr<RenderPass> m_RP_PrecomputeIrradiance;		// Diffuse 
		std::unique_ptr<RenderPass> m_RP_Prefiltering;				// Specular
		std::unique_ptr<RenderPass> m_RP_GenLUT;

	private:
		std::unique_ptr<RenderPass> SetupEquirectToCubePass();
		std::unique_ptr<RenderPass> SetupGenCubemapFromSphericalMapPass();
		std::unique_ptr<RenderPass> SetupPrecomputeIrradiancePass();	// Diffuse 
		std::unique_ptr<RenderPass> SetupPrefilteredPass();				// Specular
		std::unique_ptr<RenderPass> SetupGenLUTPass();

	private:
		// TODO: Implement a textureRef
		Texture* m_CurrSource{ nullptr };
		std::unordered_map<Texture*, IBLLuts> m_IBLLutsMap;
	};


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
		std::unique_ptr<RenderPassBase> m_RP_GenLUT;
	};
}