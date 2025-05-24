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
#include "Shader/Shader.h"

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
		m_ParamDesc.baseColor  = desc.baseColor;
		m_ParamDesc.metallic = desc.metallic;
		m_ParamDesc.roughness = desc.roughness;
		m_ParamDesc.CalDistParams(m_ParamDesc.anisotropic, m_ParamDesc.roughness, m_ParamDesc.alpha_x, m_ParamDesc.alpha_y);

		auto assetManager = g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs		  = g_RuntimeGlobalCtx.m_EntityManager;
		auto resManager   = g_RuntimeGlobalCtx.m_Resourcemanager;
		if (desc.baseColorTex) {
			m_TextureDesc.useBaseColorTex = true;
			std::shared_ptr<TextureAsset> texAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(desc.baseColorTex->GetPath()));
			std::shared_ptr<_Texture> texResource  = resManager->LoadGPUTexture(texAsset);
			m_TextureDesc.baseColorTex			   = texResource.get();
			m_ParamDesc.albedoHandle			   = texResource->GetTextureHandle();
		}
		if (desc.normalTex) {
			m_TextureDesc.useNormalMap = true;
			std::shared_ptr<TextureAsset> texAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(desc.normalTex->GetPath()));
			std::shared_ptr<_Texture> texResource = resManager->LoadGPUTexture(texAsset);
			m_TextureDesc.normalMap = texResource.get();
			m_ParamDesc.normalHandle = texResource->GetTextureHandle();
		}
		if (desc.metallicTex) {
			m_TextureDesc.useMetallicTex = true;
			std::shared_ptr<TextureAsset> texAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(desc.metallicTex->GetPath()));
			std::shared_ptr<_Texture> texResource = resManager->LoadGPUTexture(texAsset);
			m_TextureDesc.metallicTex = texResource.get();
			m_ParamDesc.metallicHandle = texResource->GetTextureHandle();
		}
		if (desc.roughnessTex) {
			m_TextureDesc.useRoughnessTex = true;
			std::shared_ptr<TextureAsset> texAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(desc.roughnessTex->GetPath()));
			std::shared_ptr<_Texture> texResource = resManager->LoadGPUTexture(texAsset);
			m_TextureDesc.roughnessTex = texResource.get();
			m_ParamDesc.roughnessHandle = texResource->GetTextureHandle();
		}
		m_ParamDesc.metallic = desc.metallic;
		m_ParamDesc.ao = desc.ao;
	}

	// Put this into renderer
	void _Material::ApplyToShader(Shader* shader, uint32_t& bindingPoint) const {
		if (m_TextureDesc.useBaseColorTex) {
			shader->SetBool("u_HasAlbedo", true);
			RenderCommand::BindTextureUnit(bindingPoint, m_TextureDesc.baseColorTex->GetTextureID());
			shader->SetInt("u_AlbedoMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasAlbedo", false);
			shader->SetVec3("u_Albedo", m_ParamDesc.baseColor);
		}

		if (m_TextureDesc.useMetallicTex) {
			shader->SetBool("u_HasMetalic", true);
			RenderCommand::BindTextureUnit(bindingPoint, m_TextureDesc.metallicTex->GetTextureID());
			shader->SetInt("u_MetalicMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasMetalic", false);
			shader->SetFloat("u_Metalic", m_ParamDesc.metallic);
		}

		if (m_TextureDesc.useRoughnessTex) {
			shader->SetBool("u_HasRoughness", true);
			RenderCommand::BindTextureUnit(bindingPoint, m_TextureDesc.roughnessTex->GetTextureID());
			shader->SetInt("u_RoughnessMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasRoughness", false);
			shader->SetFloat("u_Roughness", m_ParamDesc.roughness);
		}

		if (m_TextureDesc.useAoTex) {
			shader->SetBool("u_HasAO", true);
			RenderCommand::BindTextureUnit(bindingPoint, m_TextureDesc.aoTex->GetTextureID());
			shader->SetInt("u_AOMap", bindingPoint++);
		}
		else {
			shader->SetBool("u_HasAO", false);
			shader->SetFloat("u_AO", m_ParamDesc.ao);
		}

	}
}