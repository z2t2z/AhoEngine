#include "Ahopch.h"
#include "ShaderVariantManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"

namespace Aho {
    void ShaderVariantManager::Initialize() {
        Aho::EngineEvents::OnShaderAssetReload.AddListener(
            [this](const std::string& path, const std::shared_ptr<ShaderAsset>& asset) {
                // Inform all shaders that use this asset to recompile
                auto it = m_PathVariantCache.find(path);
                if (it != m_PathVariantCache.end()) {
                    for (auto& [feature, shader] : it->second) {
                        bool success = LoadVariant(shader, asset, (ShaderFeature)feature, false);
                        if (!success) {
                            AHO_CORE_ERROR("OnShaderAssetReload::Failed to recompile shader: {0} with feature {1}", asset->GetPath(), (uint32_t)feature);
                            continue;
                        }
                        AHO_CORE_INFO("OnShaderAssetReload::Recompiled shader: {0} with feature {1}", asset->GetPath(), (uint32_t)feature);
                    }
                }
            }
        );

        Aho::EngineEvents::OnShaderFeatureChanged.AddListener(
            [this](ShaderUsage usage, ShaderFeature featToAdd, ShaderFeature featToDel) {
                auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
                ecs->GetView<ShaderResourceComponent>().each(
                    [this, usage, featToAdd, featToDel](Entity entity, ShaderResourceComponent& shaderResource) {
                        if (shaderResource.usage == usage) {
							ShaderFeature& feat = shaderResource.feature;
                            if (!(feat & featToAdd)) {
								feat |= featToAdd;
                            }
                            if (feat & featToDel) {
								feat ^= featToDel;
                            }
                            bool success = LoadVariant(shaderResource.shader, shaderResource.shaderAsset, feat);
                            if (!success) 
                                AHO_CORE_ERROR("OnShaderFeatureChanged::LoadVariant failed for shader: {0} with feature: {1}", shaderResource.shaderAsset->GetPath(), uint32_t(feat));
                            else
                                AHO_CORE_INFO("OnShaderFeatureChanged::LoadVariant succeeded for shader: {0} with feature: {1}", shaderResource.shaderAsset->GetPath(), uint32_t(feat));
                        }
                    }
                );
            }
        );
    }

    bool ShaderVariantManager::LoadVariant(std::shared_ptr<Shader>& shader, const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature, bool checkCache) {
        if (checkCache && m_PathVariantCache.count(shaderAsset->GetPath())) {
            if (m_PathVariantCache.at(shaderAsset->GetPath()).count(uint32_t(feature))) {
                AHO_CORE_INFO("ShaderVariantManager::Find cached variant: {} with feature: {}", shaderAsset->GetName(), uint32_t(feature));
                shader = m_PathVariantCache.at(shaderAsset->GetPath()).at(uint32_t(feature));
                return true;
			} 
        }
        if (!shader) {
            shader = std::make_shared<OpenGLShader>();
        }
        std::unordered_map<uint32_t, std::string> src = shaderAsset->GetSourceCode();
		CombineSourceCodeWithVariants(src, feature);
        bool success = shader->TryCompile(src);
        if (!success) {
            AHO_CORE_ERROR("ShaderVariantManager::LoadVariant: Failed to compile shader: {} with feature: {}", shaderAsset->GetName(), uint32_t(feature));
            return false;
		}
		// Register shader 
        m_PathVariantCache[shaderAsset->GetPath()][uint32_t(feature)] = shader;

        AHO_CORE_INFO("ShaderVariantManager::Created variant for `{}` with feature: {}", shaderAsset->GetName(), uint32_t(feature));
		return true;
    }

    std::string ShaderVariantManager::GenerateDefineBlock(ShaderFeature features) const {
		std::string defineBlock;
        
        if (features & ShaderFeature::FEATURE_IBL) {
			defineBlock += "#define FEATURE_ENABLE_IBL\r\n";
        } else if (features & ShaderFeature::FEATURE_SKY_ATMOSPHERIC) {
			defineBlock += "#define FEATURE_ENABLE_SKYATMOSPHERIC\r\n";
        }

		// Add more feature defines as needed
        // ... 
        return defineBlock;
    }

    void ShaderVariantManager::CombineSourceCodeWithVariants(std::unordered_map<uint32_t, std::string>&src, ShaderFeature feature) {
        std::string defineBlock = GenerateDefineBlock(feature);
        for (auto& [type, code] : src) {
            size_t versionPos = code.find("#version");
            AHO_CORE_ASSERT(versionPos != std::string::npos, "Shader source must contain a #version directive.");

            size_t lineEnd = code.find_first_of("\r\n", versionPos);
            if (lineEnd == std::string::npos) lineEnd = code.length();

            std::string insertStr = "\n" + defineBlock + "\n";
            code.insert(lineEnd + 1, insertStr);
        }
    }
}