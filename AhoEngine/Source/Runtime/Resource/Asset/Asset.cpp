#include "Ahopch.h"
#include "Asset.h"

#include "Runtime/Function/Renderer/Texture.h"
#include "Runtime/Function/Renderer/Shader.h"

namespace Aho {
	std::string_view Aho::AssetTypeToString(AssetType type) {
		switch (type) {
			case AssetType::None:		return "AssetType::None";
			case AssetType::Scene:		return "AssetType::Scene";
			case AssetType::Texture:	return "AssetType::Texture";
			case AssetType::Mesh:		return "AssetType::Mesh";
			case AssetType::Material:	return "AssetType::Material";
		}
		return "AssetType::<Invalid>";
	}

	AssetType AssetTypeFromString(std::string_view assetType) {
		if (assetType == "AssetType::None")			return AssetType::None;
		if (assetType == "AssetType::Scene")		return AssetType::Scene;
		if (assetType == "AssetType::Texture")		return AssetType::Texture;
		if (assetType == "AssetType::Mesh")			return AssetType::Mesh;
		if (assetType == "AssetType::Material")		return AssetType::Material;

		return AssetType::None;
	}

	bool ShaderAsset::Load() {
		if (!m_Dirty) {
			return false;
		}
		m_Dirty = !m_Shader->Reload(m_Path);
		return m_Dirty;
	}

	ShaderAsset::ShaderAsset(const std::string& path, const std::shared_ptr<Shader>& shader) : Asset(path), m_Shader(shader) {
	}
} // namespace Aho