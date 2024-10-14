#include "Ahopch.h"
#include "RenderData.h"

namespace Aho {
	void RenderData::Bind(const std::shared_ptr<Shader>& shader, uint32_t texOffset) {
		shader->SetMat4("u_Model", m_Param.GetTransform()); // TODO: think of a better way
		if (m_Material) {
			m_Material->Apply(shader, texOffset);
		}
		m_VAO->Bind();
	}

	void RenderData::Unbind() {
		m_VAO->Unbind();
	}
} // namespace Aho
