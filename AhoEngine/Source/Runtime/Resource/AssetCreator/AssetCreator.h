#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include <queue>
#include <cstdlib>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aho {
	class AssetCreater {
	public:
		static std::shared_ptr<StaticMesh> MeshAssetCreater(const std::string& filePath) {
			Assimp::Importer importer;
			auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
			const aiScene* scene = importer.ReadFile(filePath, Flags);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				AHO_CORE_ERROR(importer.GetErrorString());
				return nullptr;
			}

			// TODO
			auto RetrieveMaterial = [&](aiMesh* mesh, const aiScene* scene) -> MaterialInfo {
				MaterialInfo info;
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS }) {
					for (size_t i = 0; i < material->GetTextureCount(type); i++) {
						aiString str;
						material->GetTexture(type, i, &str);
						(type == aiTextureType_NORMALS ? info.Normal : info.Albedo).push_back(std::string(str.data));
					}
				}
				return info;
			};

			std::vector<std::shared_ptr<MeshInfo>> subMesh;
			auto ProcessSubMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
				std::vector<Vertex> vertexBuffer;
				vertexBuffer.reserve(mesh->mNumVertices);
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
					vertexBuffer.push_back(vertex);
				}
				//indexBuffer.reserve(mesh->mNumVertices);
				std::vector<uint32_t> indexBuffer;
				for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
					aiFace face = mesh->mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++) {
						indexBuffer.push_back(face.mIndices[j]);
					}
				}
				subMesh.emplace_back(std::make_shared<MeshInfo>(vertexBuffer, indexBuffer, hasNormal, hasUVs, RetrieveMaterial(mesh, scene)));
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

		static std::shared_ptr<MaterialAsset> MaterialAssetCreater(const std::string& filePath) {

		}
	};

}
