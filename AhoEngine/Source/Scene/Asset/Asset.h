#pragma once

#include "Scene/UUID.h"
#include <string>

namespace Aho {
	enum class AssetType {
		None = 0,
		Scene,
		Texture,
		Material,
		Mesh
	};

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view assetType);
	//using AssetHandle = UUID;

	class Asset {
	public:
		Asset() = default;
		Asset(const std::string& path) : m_MatadataPath(path) {}
		//Asset(std::string& path) {
		//	m_MatadataPath = path;
		//}
		virtual ~Asset() = default;
		virtual AssetType GetType() const { return m_Type; }
		virtual UUID GetUUID() const { return m_Handle; }
		virtual void SetUUID(UUID id) { m_Handle = id; }
		virtual std::string_view GetMetadataPath() { return m_MatadataPath; }
		virtual std::string_view GetPath() { return m_Path; }

	protected:
		UUID m_Handle;
		AssetType m_Type;
		std::string m_MatadataPath;
		std::string m_Path;
	};

}