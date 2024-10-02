#include "Ahopch.h"
#include "AssetManagerEditor.h"
#include "Runtime/Resource/Asset/Importer/AssetImporter.h"

namespace Aho {
    //static AssetType RetrieveTypeFromPath(const std::string& path) {
    //    auto pos = path.find_last_of(".");
    //    if (pos == std::string::npos) {
    //        return AssetType::None;
    //    }
    //    std::string suffix = path.substr(pos + 1);
    //    if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "tga") {
    //        return AssetType::Texture;
    //    }
    //    if (suffix == "obj" || suffix == "fbx" || suffix == "gltf") {
    //        return AssetType::Mesh;
    //    }
    //    // TODO: Parsing internally
    //    if (suffix == "aho") {
    //        return AssetType::Scene;
    //    }
    //    if (suffix == "mat") {
    //        return AssetType::Material;
    //    }
    //    return AssetType::None;
    //}

    //std::shared_ptr<Asset> Aho::AssetManagerEditor::CreateAssetFromFile(const std::filesystem::path& filePath) {
    //    AssetType type = RetrieveTypeFromPath(filePath.string());
    //    if (type == AssetType::None) {
    //        AHO_CORE_ERROR("{Incorrect asset file type}");
    //        return nullptr;
    //    }
    //    std::shared_ptr<Asset> loadedAsset = AssetImporter::Import(type, filePath);
    //    if (!loadedAsset) {
    //        return nullptr;
    //    }
    //    auto handle = loadedAsset->GetUUID();
    //    m_Assets[handle] = loadedAsset;
    //    return loadedAsset;
    //}

    //bool AssetManagerEditor::LoadAsset(const UUID& handle) {
    //    return false;
    //}

    //bool AssetManagerEditor::UnloadAsset(const UUID& handle) {
    //    return true;
    //}

    //std::shared_ptr<Asset> AssetManagerEditor::GetAsset(const UUID& handle) const {
    //    return std::shared_ptr<Asset>();
    //}

}