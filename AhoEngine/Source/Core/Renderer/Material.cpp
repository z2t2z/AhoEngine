#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::Apply() {
        m_Shader->Bind();
        // Binding textures
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            auto type = texture->GetTextureType();
            switch (type) {
                case TextureType::Diffuse:
                    m_Shader->SetInt("u_Diffuse", i);
                case TextureType::Normal:
                    m_Shader->SetInt("u_Normal", i);
                case TextureType::Specular:
                    m_Shader->SetInt("u_Specular", i);
                case TextureType::Roughness:
                    m_Shader->SetInt("u_Roughness", i);
            }
            texture->Bind();
        }
        // Binding Parameters
        // Big TODO
        for (const auto& para : m_Parameters) {
            if (para->GetName() == "vec3") {
                m_Shader->SetVec3("u_" + para->GetName(), *static_cast<glm::vec3*>(para->GetValue()));
            }
            if (para->GetName() == "float") {
                m_Shader->SetFloat("u_" + para->GetName(), *static_cast<float*>(para->GetValue()));
            }
        }
        m_Shader->Unbind();
    }
}