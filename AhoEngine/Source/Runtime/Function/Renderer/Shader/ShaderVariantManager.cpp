#include "Ahopch.h"
#include "ShaderVariantManager.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"

namespace Aho {
    void ShaderVariantManager::Initialize() {
        
    }

    std::shared_ptr<Shader> ShaderVariantManager::GetVariant(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature) {
        if (m_PathVariantCache.count(shaderAsset->GetPath())) {
            if (m_PathVariantCache.at(shaderAsset->GetPath()).count(uint32_t(feature))) {
                return m_PathVariantCache.at(shaderAsset->GetPath()).at(uint32_t(feature));
			} 
        }

        std::unordered_map<uint32_t, std::string> src = shaderAsset->GetSourceCode();
		CombineSourceCodeWithVariants(src, feature);

        std::shared_ptr<Shader> shader = std::make_shared<OpenGLShader>();
        AHO_CORE_ASSERT(shader->TryCompile(src));

		// Register shader 
        RegisterShader(shaderAsset->GetPath(), feature, shader);

		return shader;
    }

    void ShaderVariantManager::RegisterShader(const std::string& path, ShaderFeature feature, const std::shared_ptr<Shader>& shader) {
        m_PathVariantCache[path][uint32_t(feature)] = shader;

        static bool listenerRegistered = false;
        if (listenerRegistered)
            return;
     
        listenerRegistered = true;
        Aho::EngineEvents::OnShaderAssetReload.AddListener(
            [this](const std::string& path, ShaderAsset* asset) {
				// Inform all shaders that use this asset to recompile
                auto it = m_PathVariantCache.find(path);
                if (it != m_PathVariantCache.end()) {
                    for (auto& [feature, shader] : it->second) {
						std::unordered_map<uint32_t, std::string> src = asset->GetSourceCode();
						CombineSourceCodeWithVariants(src, static_cast<ShaderFeature>(feature));
                        bool success = shader->TryCompile(src);
                        if (!success) {
                            AHO_CORE_ERROR("Failed to recompile shader: {0}", asset->GetPath());
                        } else {
                            AHO_CORE_INFO("Recompiled shader: {0}", asset->GetPath());
					    }
					}
                }
            }
        );
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

    std::string ShaderVariantManager::MakeCacheKey(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature) const {
        return shaderAsset->GetPath() + "|" + std::to_string(static_cast<uint32_t>(feature));
    }
}