#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::Unbind(const std::shared_ptr<Shader>& shader) {
        auto& chosenShader = shader == nullptr ? m_Shader : shader;
        AHO_CORE_ASSERT(chosenShader);
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(0);
        }
    }

    void Material::Apply(const std::shared_ptr<Shader>& shader) {
        auto& chosenShader = shader == nullptr ? m_Shader : shader;
        AHO_CORE_ASSERT(chosenShader);
        //chosenShader->Bind();
        // Binding textures
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            auto type = texture->GetTextureType();
            switch (type) {
                case TextureType::Diffuse:
                    chosenShader->SetInt("u_Diffuse", i);
                    break;
                case TextureType::Normal:
                    chosenShader->SetInt("u_Normal", i);
                    break;
                case TextureType::Specular:
                    chosenShader->SetInt("u_Specular", i);
                    break;
                case TextureType::Roughness:
                    chosenShader->SetInt("u_Roughness", i);
                    break;
            }
            texture->Bind(i);
        }
        // Binding Parameters
        // Big TODO
        for (const auto& para : m_Parameters) {
            if (para->GetName() == "vec3") {
                chosenShader->SetVec3("u_" + para->GetName(), *static_cast<glm::vec3*>(para->GetValue()));
            }
            if (para->GetName() == "float") {
                chosenShader->SetFloat("u_" + para->GetName(), *static_cast<float*>(para->GetValue()));
            }
        }
        //chosenShader->Unbind();
    }
}