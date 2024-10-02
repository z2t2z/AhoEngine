#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::Apply(const std::shared_ptr<Shader>& shader) {
        shader->Bind();
        // Binding textures
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            auto type = texture->GetTextureType();
            switch (type) {
                case TextureType::Diffuse:
                    shader->SetInt("u_Diffuse", i);
                    break;
                case TextureType::Normal:
                    shader->SetInt("u_Normal", i);
                    break;
                case TextureType::Specular:
                    shader->SetInt("u_Specular", i);
                    break;
                case TextureType::Roughness:
                    shader->SetInt("u_Roughness", i);
                    break;
            }
            texture->Bind();
        }
        // Binding Parameters
        // Big TODO
        for (const auto& para : m_Parameters) {
            if (para->GetName() == "vec3") {
                shader->SetVec3("u_" + para->GetName(), *static_cast<glm::vec3*>(para->GetValue()));
            }
            if (para->GetName() == "float") {
                shader->SetFloat("u_" + para->GetName(), *static_cast<float*>(para->GetValue()));
            }
        }
        //m_Shader->Unbind();
    }
}