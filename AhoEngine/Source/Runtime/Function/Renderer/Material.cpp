#include "Ahopch.h"
#include "Material.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Texture.h"
#include "Shader.h"

#include <memory>

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
		//	switch (property.m_Type) {
		//	case TexType::Albedo:
		//		std::visit([&](const auto& value) {
		//			using T = std::decay_t<decltype(value)>;
		//			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//				shader->SetBool("u_HasAlbedo", true);
		//				value->Bind(texOffset);
		//				shader->SetInt("u_AlbedoMap", texOffset++);
		//			}
		//			else if constexpr (std::is_same_v<T, glm::vec3>) {
		//				shader->SetVec3("u_RawAlbedo", value);
		//			}
		//		}, property.m_Value);
		//		break;
		//	case TexType::Normal:
		//		std::visit([&](const auto& value) {
		//			using T = std::decay_t<decltype(value)>;
		//			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//				shader->SetBool("u_HasNormal", true);
		//				value->Bind(texOffset);
		//				shader->SetInt("u_NormalMap", texOffset++);
		//			}
		//		}, property.m_Value);
		//		break;
		//	case TexType::Roughness:
		//		std::visit([&](const auto& value) {
		//			using T = std::decay_t<decltype(value)>;
		//			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//				shader->SetBool("u_HasRoughness", true);
		//				value->Bind(texOffset);
		//				shader->SetInt("u_RoughnessMap", texOffset++);
		//			}
		//			else if constexpr (std::is_same_v<T, float>) {
		//				shader->SetFloat("u_Roughness", value);
		//			}
		//		}, property.m_Value);
		//		break;
		//	case TexType::Metallic:
		//		std::visit([&](const auto& value) {
		//			using T = std::decay_t<decltype(value)>;
		//			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//				shader->SetBool("u_HasMetalic", true);
		//				value->Bind(texOffset);
		//				shader->SetInt("u_MetalicMap", texOffset++);
		//			}
		//			else if constexpr (std::is_same_v<T, float>) {
		//				shader->SetFloat("u_Metalic", value);
		//			}
		//		}, property.m_Value);
		//		break;
		//	case TexType::AO:
		//		std::visit([&](const auto& value) {
		//			using T = std::decay_t<decltype(value)>;
		//			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//				shader->SetBool("u_HasAO", true);
		//				value->Bind(texOffset);
		//				shader->SetInt("u_AOMap", texOffset++);
		//			}
		//		}, property.m_Value);
		//		break;
		//	default:
		//		//AHO_CORE_ERROR("Wrong texture type");
		//		continue;
		//	}
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

	// Use material asset(loaded from disk) to create a material that can be directly used in shader
	_Material::_Material(const std::shared_ptr<MaterialAsset>& matAsset) {
		const auto& desc  = matAsset->GetMatAssetDescriptor();
		m_Desc.baseColor  = desc.baseColor;
		auto assetManager = g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs		  = g_RuntimeGlobalCtx.m_EntityManager;
		auto resManager   = g_RuntimeGlobalCtx.m_Resourcemanager;
		if (desc.baseColorTex) {
			m_Desc.useBaseColorTex = true;
			std::shared_ptr<TextureAsset> texAsset	= assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(desc.baseColorTex->GetPath()));
			std::shared_ptr<_Texture> texResource	= resManager->LoadGPUTexture(texAsset);
			m_Desc.baseColorTex						= texResource.get();
		}
		m_Desc.metallic = desc.metallic;
		m_Desc.ao = desc.ao;
	}

	// Put this into renderer
	void _Material::ApplyToShader(Shader* shader, const _Material& mat, uint32_t& bindingPoint) {
		const auto& desc = mat.m_Desc;
		if (desc.useBaseColorTex) {
			shader->SetBool("u_HasAlbedo", true);
			RenderCommand::BindTextureUnit(bindingPoint, desc.baseColorTex->GetTextureID());
			shader->SetInt("u_AlbedoMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasAlbedo", false);
			shader->SetVec3("u_Albedo", desc.baseColor);
		}

		if (desc.useMetallicTex) {
			shader->SetBool("u_HasMetalic", true);
			RenderCommand::BindTextureUnit(bindingPoint, desc.metallicTex->GetTextureID());
			shader->SetInt("u_MetalicMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasMetalic", false);
			shader->SetFloat("u_Metalic", desc.metallic);
		}

		if (desc.useRoughnessTex) {
			shader->SetBool("u_HasRoughness", true);
			RenderCommand::BindTextureUnit(bindingPoint, desc.roughnessTex->GetTextureID());
			shader->SetInt("u_RoughnessMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasRoughness", false);
			shader->SetFloat("u_Roughness", desc.roughness);
		}

		if (desc.useAoTex) {
			shader->SetBool("u_HasAO", true);
			RenderCommand::BindTextureUnit(bindingPoint, desc.aoTex->GetTextureID());
			shader->SetInt("u_AOMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasAO", false);
			shader->SetFloat("u_AO", desc.ao);
		}

	}
}