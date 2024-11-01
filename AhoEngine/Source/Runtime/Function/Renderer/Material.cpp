#include "Ahopch.h"
#include "Material.h"

namespace Aho {
    void Material::ClearState(const std::shared_ptr<Shader>& shader) {
        for (const auto& name : { "u_HasDiffuse", "u_HasNormal", "u_HasAO" }) {
            shader->SetBool(name, false);
        }
    }
    
    void Material::Apply(const std::shared_ptr<Shader>& shader, uint32_t texOffset) {
        AHO_CORE_ASSERT(shader);
        ClearState(shader);
        for (size_t i = 0; i < m_Textures.size(); i++) {
            const auto& texture = m_Textures[i];
            texture->Bind(i + texOffset);
            auto type = texture->GetTexType();
            switch (type) {
                case TexType::Albedo:
                    shader->SetBool("u_HasDiffuse", true);
                    shader->SetInt("u_Diffuse", i + texOffset);
                    break;
                case TexType::Normal:
                    shader->SetBool("u_HasNormal", true);
                    shader->SetInt("u_Normal", i + texOffset);
                    break;
                case TexType::Specular:
                    shader->SetInt("u_Specular", i + texOffset);
                    break;
                case TexType::Roughness:
                    shader->SetInt("u_Roughness", i + texOffset);
                    break;
                case TexType::Depth:
                    shader->SetInt("u_DepthMap", i + texOffset);
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
            if (name == "u_AO") {
                shader->SetBool("u_HasAO", true); // TODO
            }
            shader->SetFloat(name, val);
        }
        for (const auto& [name, val] : m_UniformInt) {
            shader->SetInt(name, val);
        }
        for (const auto& [name, val] : m_UniformUint) {
            shader->SetUint(name, val);
        }
    }
}