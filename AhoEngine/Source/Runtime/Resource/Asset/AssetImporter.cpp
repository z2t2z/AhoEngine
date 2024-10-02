#include "Ahopch.h"
#include "AssetImporter.h"
#include <map>
#include <memory>

namespace Aho {
	using AssetImportFunction = std::function<std::shared_ptr<Asset>(const std::filesystem::path&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Mesh, MeshImporter::ImportMesh },
	};

	std::shared_ptr<Asset> AssetImporter::Import(const AssetType type, const std::filesystem::path& filePath) {
		if (!s_AssetImportFunctions.contains(type)) {
			AHO_CORE_ERROR("No importer available for asset type: {}", (uint16_t)type);
			return nullptr;
		}
		return s_AssetImportFunctions.at(type)(filePath);
	}
} // namespace Aho
