#pragma once

namespace Aho {
	enum class TextureUsage {
		Custom = 0,
		Normal,
		Height,
		AO,

		// PBR material, based on Disney principled
		BaseColor,
		Emissive,
		Subsurface,
		Metallic,
		Specular,
		SpecTint,
		Roughness,
		Anisotropic,
		Sheen,
		SheenTint,
		Clearcoat,
		ClearcoatGloss,
		SpecTrans,
		IOR,

		// Used in deferred pipeline
		Noise,
		Position,
		Depth,
		PBRCompact,	// Pack metalness, roughness, AO into rgba
		
		// Uesd in IBL
		IBL_HDR,
		IBL_CubeMap,
		IBL_CubeMapIrradiance,
		IBL_BRDF,

		// Atmospheric luts
		ATMOS_Transmittance,
		ATMOS_MultiScatt,
		ATMOS_SkyView,
		ATMOS_AreialPerspective,

		// Path Tracing
		PT_Accumulate,
	};
}
