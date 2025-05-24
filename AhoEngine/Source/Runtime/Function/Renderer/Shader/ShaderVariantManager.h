#pragma once

#include "ShaderFeatures.h"
#include <string>
#include <unordered_map>

namespace Aho {
	class Shader;
	class ShaderAsset;
	class ShaderVariantManager {
	public:
		ShaderVariantManager() = default;
		std::shared_ptr<Shader> GetVariant(const std::shared_ptr<ShaderAsset>& shaderAsset);
	private:
		std::string GenerateDefineBlock(ShaderFeature features) const;
		std::string MakeCacheKey(const std::shared_ptr<ShaderAsset>& shaderAsset) const;
	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Cache;
	};
}