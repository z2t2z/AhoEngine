#include "Ahopch.h"
#include "ShaderAsset.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"

namespace Aho {
	bool ShaderAsset::Load() {
		if (!m_Dirty) {
			return false;
		}
		m_Dirty = !m_Shader->Reload(m_Path);
		return m_Dirty;
	}

	ShaderAsset::ShaderAsset(const std::string& path, const std::shared_ptr<Shader>& shader) : Asset(path), m_Shader(shader) {
	}

	ShaderAsset::ShaderAsset(const std::string& path, const ShaderFeature& feature, const ShaderUsage& usage)
	  : Asset(path), m_Usage(usage), m_Feature(feature) {
	}
}