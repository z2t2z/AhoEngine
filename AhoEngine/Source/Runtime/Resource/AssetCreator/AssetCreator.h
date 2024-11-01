#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aho {
	namespace Utils {
		static TexType AssimpTextureConvertor(aiTextureType type) {
			switch (type) {
			case(aiTextureType_DIFFUSE):
				return TexType::Albedo;
			case(aiTextureType_NORMALS):
				return TexType::Normal;
			case(aiTextureType_HEIGHT):
				return TexType::Normal;
			case(aiTextureType_SPECULAR):
				return TexType::Specular;
			case(aiTextureType_METALNESS):
				return TexType::Metalic;
			case(aiTextureType_AMBIENT_OCCLUSION):
				return TexType::AO;
			}
			AHO_CORE_ERROR("Texture type not supported");
		}

		// Convert assimp matrix to glm matrx
		static glm::mat4 AssimpMatrixConvertor(const aiMatrix4x4& from) {
			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		static glm::vec3 AssimpVec3Convertor(const aiVector3D& vec) {
			return glm::vec3(vec.x, vec.y, vec.z);
		}

		static glm::quat AssimpQuatConvertor(const aiQuaternion& pOrientation) {
			return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
		}
	} // namespace Utils

	class AssetCreator {
	public:
		static std::shared_ptr<StaticMesh> MeshAssetCreater(const std::string& filePath);
		static std::shared_ptr<SkeletalMesh> SkeletalMeshAssetCreator(const std::string& filePath);
		static std::shared_ptr<MaterialAsset> MaterialAssetCreator(const std::string& filePath);
		static std::shared_ptr<AnimationAsset> AnimationAssetCreator(const std::string& filePath, const std::shared_ptr<SkeletalMesh>& mesh);
	private:
		static uint32_t s_MeshCnt;
	};
}
