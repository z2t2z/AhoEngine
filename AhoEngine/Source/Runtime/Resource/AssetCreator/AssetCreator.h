#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"

namespace Aho {
	class AssetCreator {
	public:
		static std::shared_ptr<StaticMesh> MeshAssetCreater(const std::string& filePath);
		static std::shared_ptr<SkeletalMesh> SkeletalMeshAssetCreator(const std::string& filePath);
		static std::shared_ptr<MaterialAsset> MaterialAssetCreator(const std::string& filePath);
	};
}
