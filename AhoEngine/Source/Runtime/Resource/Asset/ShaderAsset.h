#pragma once
#include "Asset.h"
#include "Runtime/Function/Renderer/Shader/ShaderFeatures.h"

#include <memory>

namespace Aho {
	enum class ShaderUsage;
	class Shader;
	class ShaderAsset : public Asset {
	public:
		ShaderAsset(const std::string& path, const std::shared_ptr<Shader>& shader);
		ShaderAsset(const std::string& path, const ShaderFeature& feature, const ShaderUsage& usage);
		virtual bool Load() override;
		std::shared_ptr<Shader> GetShader() const { return m_Shader; }
		ShaderFeature GetFeature() const { return m_Feature; }
		ShaderUsage GetUsage() const { return m_Usage; }
	private:
		ShaderFeature m_Feature;
		ShaderUsage m_Usage;
		std::shared_ptr<Shader> m_Shader;
	};
}
