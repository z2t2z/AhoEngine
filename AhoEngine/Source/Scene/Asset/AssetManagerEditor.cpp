#include "Ahopch.h"
#include "AssetManagerEditor.h"
#include "AssetImporter.h"

namespace Aho {
    std::shared_ptr<Asset> Aho::AssetManagerEditor::CreateAssetFromFile(const std::filesystem::path& filePath) {

        return AssetImporter::Import(AssetType::Mesh, filePath);

    }

    bool AssetManagerEditor::LoadAsset(const UUID& handle) {
        return false;
    }

    void AssetManagerEditor::UnloadAsset(const UUID& handle) {
    }

    std::shared_ptr<Asset> AssetManagerEditor::GetAsset(const UUID& handle) const {
        return std::shared_ptr<Asset>();
    }

}