#pragma once

#include "Asset.h"

#include <variant>
#include <memory>
#include <glm/glm.hpp>

namespace Aho {
    class TextureAsset;
	enum class TextureUsage;

    class MaterialAsset : public Asset {
    public:
		struct MatAssetDescriptor {
			MatAssetDescriptor() = default;
			// --- Base Color ---
			glm::vec3 baseColor = { 1.0f,1.0f,1.0f };
			std::shared_ptr<TextureAsset> baseColorTex = nullptr;

			std::shared_ptr<TextureAsset> normalTex = nullptr;
			// --- Metallic (scalar) ---
			float metallic = 0.1f;
			std::shared_ptr<TextureAsset> metallicTex = nullptr;

			// --- Roughness (scalar) ---
			float roughness = 0.6f;
			std::shared_ptr<TextureAsset> roughnessTex = nullptr;

			// --- Ambient Occlusion (scalar) ---
			float ao = 0.0f;
			std::shared_ptr<TextureAsset> aoTex = nullptr;

			// --- Emissive Color ---
			glm::vec3 emissive = { 0.0f, 0.0f, 0.0f };
			std::shared_ptr<TextureAsset> emissiveTex = nullptr;
		};
        MaterialAsset() = default;
		MaterialAsset(const std::string& path, const std::vector<std::pair<TextureUsage, std::string>>& paths);
        MaterialAsset(const MatAssetDescriptor& desc)
            : m_Desc(desc) {
        }
        const MatAssetDescriptor& GetMatAssetDescriptor() const { return m_Desc; }
    private:
        MatAssetDescriptor m_Desc;
    };
}