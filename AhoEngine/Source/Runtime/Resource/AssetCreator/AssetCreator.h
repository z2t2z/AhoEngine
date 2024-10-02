#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/Serializer/Serializer.h"
#include <queue>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aho {
	class AssetCreater {
	public:
		static bool MeshAssetCreater(std::string& filePath) {
			Assimp::Importer importer;
			auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
			const aiScene* scene = importer.ReadFile(filePath, Flags);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				AHO_CORE_ERROR(importer.GetErrorString());
				return false;
			}
			auto ProcessSubMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
				int normal_cnt = 0;
				int texcoords_cnt = 0;
				RawMesh rawMesh;
				auto& vertexBuffer = rawMesh.vertexBuffer;
				for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
					Vertex vertex;
					vertex.x = mesh->mVertices[i].x;
					vertex.y = mesh->mVertices[i].y;
					vertex.z = mesh->mVertices[i].z;
					// Normals
					if (mesh->HasNormals()) {
						vertex.nx = mesh->mNormals[i].x;
						vertex.ny = mesh->mNormals[i].y;
						vertex.nz = mesh->mNormals[i].z;
					}
					// Texture coords, tangent and bitangent
					if (mesh->HasTextureCoords(0)) {
						vertex.u = mesh->mTextureCoords[0][i].x;
						vertex.v = mesh->mTextureCoords[0][i].y;
						vertex.tx = mesh->mTangents[i].x;
						vertex.ty = mesh->mTangents[i].y;
						vertex.tz = mesh->mTangents[i].z;
						vertex.btx = mesh->mTangents[i].x;
						vertex.bty = mesh->mTangents[i].y;
						vertex.btz = mesh->mTangents[i].z;
					}
					vertexBuffer.push_back(vertex);
				}
				auto& indexBuffer = rawMesh.indexBuffer;
				for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
					aiFace face = mesh->mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++) {
						indexBuffer.push_back(face.mIndices[j]);
					}
				}
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

			return ProcessNode(scene->mRootNode, scene);
		}
	};

}
