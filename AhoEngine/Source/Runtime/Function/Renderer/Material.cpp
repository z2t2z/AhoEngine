#include "Ahopch.h"
#include "Material.h"

namespace Aho {
	// TODO: Try to come up with a better way
    void Material::ClearState(const std::shared_ptr<Shader>& shader) {
        for (const auto& name : { "u_HasAlbedo", "u_HasNormal", "u_HasAO", "u_HasMetallic", "u_HasRoughness" }) {
            shader->SetBool(name, false);
        }
    }

    void Material::Apply(const std::shared_ptr<Shader>& shader, uint32_t& texOffset) {
		AHO_CORE_ASSERT(shader);
		ClearState(shader);
		for (const auto& property : m_Properties) {
			switch (property.m_Type) {
			case TexType::Albedo:
				std::visit([&](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
						shader->SetBool("u_HasAlbedo", true);
						value->Bind(texOffset);
						shader->SetInt("u_AlbedoMap", texOffset++);
					}
					else if constexpr (std::is_same_v<T, glm::vec3>) {
						shader->SetVec3("u_RawAlbedo", value);
					}
				}, property.m_Value);
				break;
			case TexType::Normal:
				std::visit([&](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
						shader->SetBool("u_HasNormal", true);
						value->Bind(texOffset);
						shader->SetInt("u_NormalMap", texOffset++);
					}
				}, property.m_Value);
				break;
			case TexType::Roughness:
				std::visit([&](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
						shader->SetBool("u_HasRoughness", true);
						value->Bind(texOffset);
						shader->SetInt("u_RoughnessMap", texOffset++);
					}
					else if constexpr (std::is_same_v<T, float>) {
						shader->SetFloat("u_Roughness", value);
					}
				}, property.m_Value);
				break;
			case TexType::Metallic:
				std::visit([&](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
						shader->SetBool("u_HasMetalic", true);
						value->Bind(texOffset);
						shader->SetInt("u_MetalicMap", texOffset++);
					}
					else if constexpr (std::is_same_v<T, float>) {
						shader->SetFloat("u_Metalic", value);
					}
				}, property.m_Value);
				break;
			case TexType::AO:
				std::visit([&](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
						shader->SetBool("u_HasAO", true);
						value->Bind(texOffset);
						shader->SetInt("u_AOMap", texOffset++);
					}
				}, property.m_Value);
				break;
			default:
				AHO_CORE_ERROR("Wrong texture type");
				continue;
			}
		}
    }

	bool Material::HasProperty(TexType type) {
		auto it = std::find_if(m_Properties.begin(), m_Properties.end(), [&](const auto& prop) {
			return prop.m_Type == type;
		});
		return it != m_Properties.end();
	}

	const MaterialProperty& Material::GetProperty(TexType type) {
		auto it = std::find_if(m_Properties.begin(), m_Properties.end(), [&](const auto& prop) {
			return prop.m_Type == type;
		});
		AHO_CORE_ASSERT(it != m_Properties.end());
		return *it;
	}
}