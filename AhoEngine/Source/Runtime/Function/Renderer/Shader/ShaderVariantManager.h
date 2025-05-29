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
		std::shared_ptr<Shader> GetVariant(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature);
	private:
		void RegisterShader(const std::string& path, ShaderFeature feature, const std::shared_ptr<Shader>& shader);
		std::string GenerateDefineBlock(ShaderFeature features) const;
		std::string MakeCacheKey(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature) const;
		void CombineSourceCodeWithVariants(std::unordered_map<uint32_t, std::string>& src, ShaderFeature feature);
	private:
		std::unordered_map<std::string, std::unordered_map<uint32_t, std::shared_ptr<Shader>>> m_PathVariantCache;
	};
}