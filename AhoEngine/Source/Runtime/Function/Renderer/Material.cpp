#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::UnbindTexture() {
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(0);
        }
    }

    void Material::Apply(const std::shared_ptr<Shader>& shader) {
        AHO_CORE_ASSERT(shader);
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(i);
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
                case TextureType::Depth:
                    shader->SetInt("u_Depth", i);
                    break;
                default:
                    AHO_CORE_ERROR("Wrong texture type");
                    continue;
            }
        }
        for (const auto& [name, val] : m_UniformVec3) {
            shader->SetVec3(name, val);
        }
        for (const auto& [name, val] : m_UniformMat4) {
            shader->SetMat4(name, val);
        }
        for (const auto& [name, val] : m_UniformFloat) {
            shader->SetFloat(name, val);
        }
        for (const auto& [name, val] : m_UniformInt) {
            shader->SetInt(name, val);
        }
        for (const auto& [name, val] : m_UniformUint) {
            shader->SetUint(name, val);
        }
        //chosenShader->Unbind(); // !!!!!!???
    }

}