#include "Ahopch.h"
#include "DisneyPrincipled.h"

namespace Aho {
	std::unordered_map<TexType, std::string> TextureHandles::s_Umap = {
		{TexType::Albedo, "Base color"},
		{TexType::Emissive, "Emissive"},
		{TexType::EmissiveScale, "Emissive Intensity"},
		{TexType::Subsurface, "Subsurface"},
		{TexType::Metallic, "Metallic"},
		{TexType::Specular, "Specular"},
		{TexType::SpecTint, "SpecTint"},
		{TexType::Roughness, "Roughness"},
		{TexType::Anisotropic, "Anisotropic"},
		{TexType::Sheen, "Sheen"},
		{TexType::SheenTint, "SheenTint"},
		{TexType::Clearcoat, "Clearcoat"},
		{TexType::ClearcoatGloss, "Clearcoat Gloss"},
		{TexType::SpecTrans, "Specular Transmission"},
		{TexType::ior, "Index of refraction"},

		{TexType::Normal, "Normal"},
		{TexType::AO, "Ambient occlusion"},
	};
}
