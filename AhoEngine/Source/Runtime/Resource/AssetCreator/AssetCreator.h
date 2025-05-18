#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aho {
	class AssetCreator {
	public:
		static std::shared_ptr<StaticMesh> MeshAssetCreater(const std::string& filePath, const glm::mat4& preTransform = glm::mat4(1.0f));
		static std::shared_ptr<SkeletalMesh> SkeletalMeshAssetCreator(const std::string& filePath, const glm::mat4& preTransform = glm::mat4(1.0f));
		static std::shared_ptr<MaterialAsset> MaterialAssetCreator(const std::string& filePath);
		static std::shared_ptr<AnimationAsset> AnimationAssetCreator(const std::string& filePath, const std::shared_ptr<SkeletalMesh>& mesh);
	private:
		static uint32_t s_MeshCnt;
	};
}
