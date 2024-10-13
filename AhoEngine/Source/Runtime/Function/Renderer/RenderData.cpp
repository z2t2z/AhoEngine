#include "Ahopch.h"
#include "RenderData.h"

namespace Aho {
	void RenderData::Bind(const std::shared_ptr<Shader>& shader) {
		m_VAO->Bind();
		AHO_CORE_ASSERT(m_Para);
		shader->SetMat4("u_Model", m_Para->GetTransform()); // TODO: think of a better way
		if (m_BindMaterial && m_Material) {
			m_Material->Apply(shader);
		}
	}

	void RenderData::Unbind() {
		m_VAO->Unbind();
		if (m_BindMaterial && m_Material) {
			m_Material->UnbindTexture();
		}
	}
} // namespace Aho
