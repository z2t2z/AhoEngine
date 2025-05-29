#pragma once
#include "Asset.h"
#include "Runtime/Function/Renderer/Shader/ShaderFeatures.h"
#include <memory>

namespace Aho {
	class Shader;
	class ShaderAsset : public Asset {
	public:
		ShaderAsset(const std::string& path, const ShaderUsage& usage = ShaderUsage::None);
		virtual bool Load() override;
		ShaderUsage GetUsage() const { return m_Usage; }
		std::unordered_map<uint32_t, std::string> GetSourceCode() const { return m_OpenGLSourceCode; }
	private:
		std::string ReadFile(const std::string& path);
		void Preprocess(std::unordered_map<uint32_t, std::string>& umap, const std::string& src);
		void ReplaceIncludes(std::unordered_map<uint32_t, std::string>& umap);
	private:
		ShaderUsage m_Usage;
		ShaderType m_Type;
		std::unordered_map<uint32_t, std::string> m_OpenGLSourceCode;
	};
}
