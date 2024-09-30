#pragma once
#include "Asset.h"
#include <memory>
#include <filesystem>
#include <map>

namespace Aho {
    class AssetManagerBase {
    public:
        virtual ~AssetManagerBase() = default;

        virtual std::shared_ptr<Asset> CreateAssetFromFile(const std::filesystem::path& filePath) = 0;
        virtual bool LoadAsset(const UUID& handle) = 0;
        virtual void UnloadAsset(const UUID& handle) = 0;
        virtual std::shared_ptr<Asset> GetAsset(const UUID& handle) const = 0;

    protected:
        std::map<UUID, std::shared_ptr<Asset>> m_Assets;
        std::map<UUID, std::vector<UUID>> m_Dependencies; // DAG
    };
}

