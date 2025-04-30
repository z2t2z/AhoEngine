#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {

	struct AtmosphereParameters {
		// Radius of the planet (center to ground)
		float BottomRadius;
		// Maximum considered atmosphere height (center to atmosphere top)
		float TopRadius;

		// Rayleigh scattering exponential distribution scale in the atmosphere
		float RayleighDensityExpScale;
		// Rayleigh scattering coefficients
		glm::vec3 RayleighScattering;

		// Mie scattering exponential distribution scale in the atmosphere
		float MieDensityExpScale;
		// Mie scattering coefficients
		glm::vec3 MieScattering;
		// Mie extinction coefficients
		glm::vec3 MieExtinction;
		// Mie absorption coefficients
		glm::vec3 MieAbsorption;
		// Mie phase function excentricity
		float MiePhaseG;

		// An atmosphere layer of width 'width', and whose density is defined as
		// 'ExpTerm' * exp('ExpScale' * h) + 'LinearTerm' * h + 'ConstantTerm',
		// clamped to [0,1], and where h is the altitude.	
		// Refer to Bruneton's implementation of definitions.glsl for more details
		// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/183ead5bdacc701b3b626347a680a2f3cd3d4fbd/Resources/Bruneton17/definitions.glsl
		glm::vec3 AbsorptionExtinction;
		float Width0;
		float ExpTerm0;
		float ExpScale0;
		float LinearTerm0;
		float ConstantTerm0;

		float Width1;
		float ExpTerm1;
		float ExpScale1;
		float LinearTerm1;
		float ConstantTerm1;

		// The albedo of the ground.
		glm::vec3 GroundAlbedo;
	};

	class RenderSkyPipeline : public RenderPipeline {
	public:
		RenderSkyPipeline();
		~RenderSkyPipeline() = default;
		virtual void Initialize() override;

		glm::vec3 GetSunDir() { return m_SunDir; }
		void SetSunDir(const glm::vec3& dir) { m_SunDir = dir; }
		AtmosphereParameters& GetAtmosphereParams() { return m_AtmosParams; }

	private:
		std::unique_ptr<RenderPass> SetupTransmittanceLUTPass();
		std::unique_ptr<RenderPass> SetupMutiScattLutPass();
		std::unique_ptr<RenderPass> SetupSkyViewLutPass();

	private:
		glm::vec3 m_SunColor{ 1 };
		float m_SunIntensity{ 1 };
		glm::vec3 m_SunDir;
		AtmosphereParameters m_AtmosParams;
	
	private:
		std::unique_ptr<RenderPass> m_TransmittanceLutPass;
		std::unique_ptr<RenderPass> m_MutiScattLutPass;
		std::unique_ptr<RenderPass> m_SkyViewLutPass;

	};
}
