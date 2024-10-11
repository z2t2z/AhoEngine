#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::UnbindTexture(const std::shared_ptr<Shader>& shader) {
        auto& chosenShader = shader == nullptr ? m_Shader : shader;
        AHO_CORE_ASSERT(chosenShader);
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(0);
        }
    }

    void Material::Apply(const std::shared_ptr<Shader>& shader) {
        const auto& chosenShader = shader == nullptr ? m_Shader : shader;
        AHO_CORE_ASSERT(chosenShader);
        chosenShader->Bind();
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(i);
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
                default:
                    AHO_CORE_ERROR("Wrong texture type");
                    continue;
            }
        }
        for (const auto& [name, val] : m_UniformVec3) {
            chosenShader->SetVec3(name, val);
        }
        for (const auto& [name, val] : m_UniformMat4) {
            chosenShader->SetMat4(name, val);
        }
        for (const auto& [name, val] : m_UniformFloat) {
            chosenShader->SetFloat(name, val);
        }
        for (const auto& [name, val] : m_UniformInt) {
            chosenShader->SetInt(name, val);
        }
        for (const auto& [name, val] : m_UniformUint) {
            chosenShader->SetUint(name, val);
        }
        //chosenShader->Unbind(); // !!!!!!???
    }

}