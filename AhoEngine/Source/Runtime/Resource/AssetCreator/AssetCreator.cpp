#include "Ahopch.h"
#include "AssetCreator.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <queue>
#include <cstdlib>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aho {
	namespace Utils {
		static TextureType AssimpTextureConvertor(aiTextureType type) {
			switch (type) {
				case(aiTextureType_DIFFUSE):
					return TextureType::Diffuse;
				case(aiTextureType_NORMALS):
					return TextureType::Normal;
				case(aiTextureType_HEIGHT):
					return TextureType::Normal;
				case(aiTextureType_SPECULAR):
					return TextureType::Specular;
				case(aiTextureType_METALNESS):
					return TextureType::Metalic;
				case(aiTextureType_AMBIENT_OCCLUSION):
					return TextureType::AO;
			}
			AHO_CORE_ERROR("Texture type not supported");
		}

		// Convert assimp matrix to glm matrx
		static glm::mat4 MatrixConvertor(const aiMatrix4x4& from) {
			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		static glm::vec3 Vec3Convertor(const aiVector3D& vec) {
			return glm::vec3(vec.x, vec.y, vec.z);
		}

		static glm::quat QuatConvertor(const aiQuaternion& pOrientation) {
			return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
		}
	} // namespace Utils

	std::shared_ptr<StaticMesh> AssetCreator::MeshAssetCreater(const std::string& filePath) {
		auto it = filePath.find_last_of('/\\');
		std::string prefix;
		if (it != std::string::npos) {
			prefix = filePath.substr(0, it) + '/';
		}
		Assimp::Importer importer;
		// TODO: flags can be customized in editor
		auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
		const aiScene* scene = importer.ReadFile(filePath, Flags);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			AHO_CORE_ERROR(importer.GetErrorString());
			return nullptr;
		}

		// TODO
		auto RetrieveMaterial = [&](aiMesh* mesh, const aiScene* scene) -> MaterialInfo {
			MaterialInfo info;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_HEIGHT, aiTextureType_SPECULAR, aiTextureType_METALNESS, aiTextureType_AMBIENT_OCCLUSION }) {
				for (size_t i = 0; i < material->GetTextureCount(type); i++) {
					aiString str;
					material->GetTexture(type, i, &str);
					info.materials.emplace_back(Utils::AssimpTextureConvertor(type), prefix + std::string(str.data));
				}
			}
			return info;
		};

		std::vector<std::shared_ptr<MeshInfo>> subMesh;
		auto ProcessSubMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
			std::vector<Vertex> vertices;
			vertices.reserve(mesh->mNumVertices);
			bool hasNormal = mesh->HasNormals();
			bool hasUVs = mesh->HasTextureCoords(0);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				vertex.x = mesh->mVertices[i].x;
				vertex.y = mesh->mVertices[i].y;
				vertex.z = mesh->mVertices[i].z;
				// Normals
				if (hasNormal) {
					vertex.nx = mesh->mNormals[i].x;
					vertex.ny = mesh->mNormals[i].y;
					vertex.nz = mesh->mNormals[i].z;
				}
				// Texture coords, tangent and bitangent
				if (hasUVs) {
					vertex.u = mesh->mTextureCoords[0][i].x;
					vertex.v = mesh->mTextureCoords[0][i].y;
					vertex.tx = mesh->mTangents[i].x;
					vertex.ty = mesh->mTangents[i].y;
					vertex.tz = mesh->mTangents[i].z;
					vertex.btx = mesh->mBitangents[i].x;
					vertex.bty = mesh->mBitangents[i].y;
					vertex.btz = mesh->mBitangents[i].z;
				}
				vertices.push_back(vertex);
			}
			std::vector<uint32_t> indexBuffer;
			indexBuffer.reserve(mesh->mNumVertices);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) {
					indexBuffer.push_back(face.mIndices[j]);
				}
			}
			subMesh.emplace_back(std::make_shared<MeshInfo>(vertices, indexBuffer, hasNormal, hasUVs, RetrieveMaterial(mesh, scene)));
			return true;
		};

		auto ProcessNode = [&](aiNode* node, const aiScene* scene) -> bool {
			std::queue<aiNode*> q;
			q.push(node);
			while (!q.empty()) {
				aiNode* cur_node = q.front();
				q.pop();
				for (uint32_t i = 0; i < cur_node->mNumMeshes; i++) {
					aiMesh* mesh = scene->mMeshes[cur_node->mMeshes[i]];
					if (!ProcessSubMesh(mesh, scene)) {
						return false;
					}
				}
				for (uint32_t i = 0; i < cur_node->mNumChildren; i++) {
					q.push(cur_node->mChildren[i]);
				}
			}
			return true;
		};
		ProcessNode(scene->mRootNode, scene);

		return std::make_shared<StaticMesh>(subMesh);
	}

	std::shared_ptr<SkeletalMesh> AssetCreator::SkeletalMeshAssetCreator(const std::string& filePath) {
		Assimp::Importer importer;
		// TODO: flags can be customized in editor
		auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
		const aiScene* scene = importer.ReadFile(filePath, Flags);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			AHO_CORE_ERROR(importer.GetErrorString());
			return nullptr;
		}

		// TODO
		auto RetrieveMaterial = [&](aiMesh* mesh, const aiScene* scene) -> MaterialInfo {
			MaterialInfo info;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_SPECULAR, aiTextureType_METALNESS, aiTextureType_AMBIENT_OCCLUSION }) {
				for (size_t i = 0; i < material->GetTextureCount(type); i++) {
					aiString str;
					material->GetTexture(type, i, &str);
					info.materials.emplace_back(Utils::AssimpTextureConvertor(type), std::string(str.data));
				}
			}
			return info;
		};

		std::vector<std::shared_ptr<SkeletalMeshInfo>> subMesh;
		std::map<std::string, BoneInfo> boneCache;
		uint32_t bonesCnt = 0;
		auto RetrieveSkeletonInfo = [&](std::vector<VertexSkeletal>& vertices, aiMesh* mesh, const aiScene* scene) -> void {
			for (uint32_t i = 0; i < mesh->mNumBones; i++) {
				std::string boneName(mesh->mBones[i]->mName.C_Str());
				if (!boneCache.contains(boneName)) {
					BoneInfo info(bonesCnt++, boneName, Utils::MatrixConvertor(mesh->mBones[i]->mOffsetMatrix));
					boneCache[boneName] = info;
				}
				int id = boneCache.at(boneName).id;
				auto weights = mesh->mBones[i]->mWeights;
				int numWeights = mesh->mBones[i]->mNumWeights;
				for (uint32_t j = 0; j < numWeights; j++) {
					int vertexID = weights[j].mVertexId;
					float weight = weights[j].mWeight;
					AHO_CORE_ASSERT(vertexID < vertices.size(), "Incorrect skeletal mesh data");
					VertexSkeletal& vertex = vertices[vertexID];
					for (int k = 0; k < MAX_BONES; k++) {
						if (vertex.bonesID[k] == -1) {
							vertex.weights[k] = weight;
							vertex.bonesID[k] = id;
							break;
						}
					}
				}
			}
		};

		auto ProcessSubMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
			std::vector<VertexSkeletal> vertices;
			vertices.reserve(mesh->mNumVertices);
			bool hasNormal = mesh->HasNormals();
			bool hasUVs = mesh->HasTextureCoords(0);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
				VertexSkeletal vertex;
				vertex.x = mesh->mVertices[i].x;
				vertex.y = mesh->mVertices[i].y;
				vertex.z = mesh->mVertices[i].z;
				// Normals
				if (hasNormal) {
					vertex.nx = mesh->mNormals[i].x;
					vertex.ny = mesh->mNormals[i].y;
					vertex.nz = mesh->mNormals[i].z;
				}
				// Texture coords, tangent and bitangent
				if (hasUVs) {
					vertex.u = mesh->mTextureCoords[0][i].x;
					vertex.v = mesh->mTextureCoords[0][i].y;
					vertex.tx = mesh->mTangents[i].x;
					vertex.ty = mesh->mTangents[i].y;
					vertex.tz = mesh->mTangents[i].z;
					vertex.btx = mesh->mBitangents[i].x;
					vertex.bty = mesh->mBitangents[i].y;
					vertex.btz = mesh->mBitangents[i].z;
				}
				vertices.push_back(vertex);
			}
			RetrieveSkeletonInfo(vertices, mesh, scene);

			std::vector<uint32_t> indexBuffer;
			indexBuffer.reserve(mesh->mNumVertices);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) {
					indexBuffer.push_back(face.mIndices[j]);
				}
			}
			subMesh.emplace_back(std::make_shared<SkeletalMeshInfo>(vertices, indexBuffer, hasNormal, hasUVs, RetrieveMaterial(mesh, scene)));
			return true;
		};

		auto ProcessNode = [&](aiNode* node, const aiScene* scene) -> bool {
			std::queue<aiNode*> q;
			q.push(node);
			while (!q.empty()) {
				aiNode* cur_node = q.front();
				q.pop();
				for (uint32_t i = 0; i < cur_node->mNumMeshes; i++) {
					aiMesh* mesh = scene->mMeshes[cur_node->mMeshes[i]];
					if (!ProcessSubMesh(mesh, scene)) {
						return false;
					}
				}
				for (uint32_t i = 0; i < cur_node->mNumChildren; i++) {
					q.push(cur_node->mChildren[i]);
				}
			}
			return true;
		};
		ProcessNode(scene->mRootNode, scene);
		return std::make_shared<SkeletalMesh>(subMesh);
	}

	std::shared_ptr<MaterialAsset> AssetCreator::MaterialAssetCreator(const std::string& filePath) {
		return std::make_shared<MaterialAsset>();
	}
} // namespace Aho