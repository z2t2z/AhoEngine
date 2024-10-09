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
        //if (!m_Outdated) {
        //    return;
        //}
        //m_Outdated = false;
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
        //chosenShader->Unbind(); // !!!!!!???
    }

    template<>
    inline void Material::SetUniform<glm::vec3>(const std::string& name, const glm::vec3& value) {
        m_Outdated = true;
        m_UniformVec3[name] = value;
    }
    template<>
    inline void Material::SetUniform<glm::mat4>(const std::string& name, const glm::mat4& value) {
        m_Outdated = true;
        m_UniformMat4[name] = value;
    }
    template<>
    inline void Material::SetUniform<float>(const std::string& name, const float& value) {
        m_Outdated = true;
        m_UniformFloat[name] = value;
    }
    template<>
    inline void Material::SetUniform<int>(const std::string& name, const int& value) {
        m_Outdated = true;
        m_UniformInt[name] = value;
    }
}