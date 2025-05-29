#pragma once

#include <memory>

namespace Aho {
	class _Texture;
	class TextureAsset;
	class Asset;
	class VertexArray;
	class MeshAsset;
	class Entity;
	class Shader;
	class ShaderAsset;
	class ShaderVariantManager;
	enum class ShaderFeature : uint32_t;

	// Responsible for loading and caching runtime resources from assets which are loaded from disk
	class ResourceManager {
	public:
		ResourceManager() { Initialize(); }
		void Initialize();
		std::shared_ptr<_Texture> LoadGPUTexture(const std::shared_ptr<TextureAsset>& textureAsset);
		std::shared_ptr<VertexArray> LoadVAO(const std::shared_ptr<MeshAsset>& textureAsset);
		std::shared_ptr<Shader> LoadShaderResource(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature);
	public:
		// Should not be here
		Entity LoadIBL(std::shared_ptr<TextureAsset>& textureAsset);
		Entity CreateGameObject(const std::string& name);
	private:
		std::unique_ptr<ShaderVariantManager> m_ShaderVariantManager;
		std::unordered_map<std::string, int> m_GameObjects;
		std::unordered_map<std::shared_ptr<Asset>, std::shared_ptr<_Texture>> m_TextureCached;
		std::unordered_map<std::shared_ptr<Asset>, std::shared_ptr<VertexArray>> m_VAOCached;
	};
}
