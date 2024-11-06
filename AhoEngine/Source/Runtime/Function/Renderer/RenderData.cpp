#include "Ahopch.h"
#include "RenderData.h"

namespace Aho {
	// TODO: think of a better way
	void RenderData::Bind(const std::shared_ptr<Shader>& shader, uint32_t texOffset) {
		if (m_Param) {
			shader->SetMat4("u_Model", m_Param->GetTransform()); 
		}
		if (m_BoneOffset != -1) {
			shader->SetInt("u_BoneOffset", m_BoneOffset);
		}
		if (m_Material) {
			m_Material->Apply(shader, texOffset);
		}
		m_VAO->Bind();
	}

	void RenderData::Unbind() {
		m_VAO->Unbind();
	}
} // namespace Aho
