#include "Ahopch.h"
#include "RenderData.h"

namespace Aho {
	void RenderData::Bind() {
		m_VAO->Bind();
		AHO_CORE_ASSERT(m_Para);
		if (m_Material) {
			auto shader = m_Material->GetShader();
			shader->SetMat4("u_Model", m_Para->GetTransform()); // TODO: think of a better way
			m_Material->Apply();
		}
	}

	void RenderData::Unbind() {
		m_VAO->Unbind();
		if (m_Material) {
			m_Material->UnbindTexture();
		}
	}
} // namespace Aho
