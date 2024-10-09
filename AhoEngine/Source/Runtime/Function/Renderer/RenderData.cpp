#include "Ahopch.h"
#include "RenderData.h"

namespace Aho {
	void RenderData::Bind() {
		m_VAO->Bind();
		if (m_Material) {
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
