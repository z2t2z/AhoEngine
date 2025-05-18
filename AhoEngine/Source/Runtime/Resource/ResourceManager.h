#pragma once

#include <memory>

namespace Aho {
	class _Texture;
	class TextureAsset;
	class Asset;
	class VertexArray;
	class MeshAsset;
	class Entity;

	class ResourceManager {
	public:
		ResourceManager() = default;
		std::shared_ptr<_Texture> LoadGPUTexture(const std::shared_ptr<TextureAsset>& textureAsset);
		std::shared_ptr<VertexArray> LoadVAO(const std::shared_ptr<MeshAsset>& textureAsset);
		Entity CreateGameObject(const std::string& name);
	private:
		std::unordered_map<std::string, int> m_GameObjects;
		std::unordered_map<std::shared_ptr<Asset>, std::shared_ptr<_Texture>> m_TextureCached;
		std::unordered_map<std::shared_ptr<Asset>, std::shared_ptr<VertexArray>> m_VAOCached;
	};
}
