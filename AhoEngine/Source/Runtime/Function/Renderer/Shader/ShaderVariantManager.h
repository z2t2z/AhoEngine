#pragma once

#include "ShaderFeatures.h"
#include <string>
#include <unordered_map>

namespace Aho {
	class Shader;
	class ShaderAsset;

	// Load shader variant resources from shader assets
	class ShaderVariantManager {
	public:
		ShaderVariantManager() = default;
		void Initialize();
		bool LoadVariant(std::shared_ptr<Shader>& shader, const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature, bool checkCache = true);
	private:
		std::string GenerateDefineBlock(ShaderFeature features) const;
		void CombineSourceCodeWithVariants(std::unordered_map<uint32_t, std::string>& src, ShaderFeature feature);
	private:
		std::unordered_map<std::string, std::unordered_map<uint32_t, std::shared_ptr<Shader>>> m_PathVariantCache;
	};
}