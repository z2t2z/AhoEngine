#include "Ahopch.h"
#include "ShaderVariantManager.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"

namespace Aho {
    std::shared_ptr<Shader> ShaderVariantManager::GetVariant(const std::shared_ptr<ShaderAsset>& shaderAsset) {
        std::string cacheKey = MakeCacheKey(shaderAsset);
        if (m_Cache.count(cacheKey)) {
            return m_Cache.at(cacheKey);
        }

        return std::make_shared<OpenGLShader>(shaderAsset->GetPath());
    }

    std::string ShaderVariantManager::GenerateDefineBlock(ShaderFeature features) const {
        return std::string();
    }

    std::string Aho::ShaderVariantManager::MakeCacheKey(const std::shared_ptr<ShaderAsset>& shaderAsset) const {
        return shaderAsset->GetPath() + "|" + std::to_string(static_cast<uint32_t>(shaderAsset->GetFeature()));
    }
}