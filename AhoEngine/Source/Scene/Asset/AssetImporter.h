#pragma once

#include "Asset.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "SceneAsset.h"
#include "MaterialAsset.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>

namespace Aho {
	class AssetImporter {
	public:
		static std::shared_ptr<Asset> Import(const AssetType type, const std::filesystem::path& filePath);
	};


	static class MeshImporter {
		using VertexArrayList = std::vector<std::shared_ptr<VertexArray>>;
		using MeterialList = std::vector<std::shared_ptr<Material>>;
	public:
		static std::shared_ptr<Asset>ImportMesh(const std::filesystem::path& filePath) {

			VertexArrayList VAOs;
			MeterialList Materials;

			if (!LoadModel(filePath, VAOs, Materials)) {
				AHO_CORE_ERROR("Failed to load mesh");
				return nullptr;
			}
			std::shared_ptr<MeshAsset> meshAsset = std::make_shared<MeshAsset>(filePath.string(), std::move(VAOs), std::move(Materials));
			return meshAsset;
		}
	private:
		static bool LoadModel(const std::filesystem::path& filePath, VertexArrayList& VAOs, MeterialList& Materials) {
			Assimp::Importer importer;
			auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
			const aiScene* scene = importer.ReadFile(filePath.string(), Flags);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				AHO_CORE_ERROR(importer.GetErrorString());
				return false;
			}

			auto ProcessMaterials = [&]() -> bool {
				return false;
			};

			auto ProcessMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
				std::vector<float> vertices;
				std::shared_ptr<VertexArray> vertexArray;
				vertexArray.reset(VertexArray::Create());
				for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
					// Position
					vertices.push_back(mesh->mVertices[i].x);
					vertices.push_back(mesh->mVertices[i].y);
					vertices.push_back(mesh->mVertices[i].z);
					// Normals
					if (mesh->HasNormals()) {
						vertices.push_back(mesh->mNormals[i].x);
						vertices.push_back(mesh->mNormals[i].y);
						vertices.push_back(mesh->mNormals[i].z);
					}
					// Texture coords, tangent and bitangent
					if (mesh->HasTextureCoords(0)) {
						vertices.push_back(mesh->mTextureCoords[0][i].x);
						vertices.push_back(mesh->mTextureCoords[0][i].y);
						vertices.push_back(mesh->mTangents[i].x);
						vertices.push_back(mesh->mTangents[i].y);
						vertices.push_back(mesh->mTangents[i].z);
						vertices.push_back(mesh->mBitangents[i].x);
						vertices.push_back(mesh->mBitangents[i].y);
						vertices.push_back(mesh->mBitangents[i].z);
					}
				}
				std::shared_ptr<VertexBuffer> vertexBuffer;
				vertexBuffer.reset(VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float)));
				BufferLayout layout = {
					{ ShaderDataType::Float3, "a_Position" }
				};
				if (mesh->HasNormals()) {
					layout.Push({ ShaderDataType::Float3, "a_Normal" });
				}
				if (mesh->HasTextureCoords(0)) {
					layout.Push({ ShaderDataType::Float2, "a_TexCoords" });
					layout.Push({ ShaderDataType::Float3, "a_Tangent" });
					layout.Push({ ShaderDataType::Float3, "a_Bitangent" });
				}
				vertexBuffer->SetLayout(layout);
				vertexArray->AddVertexBuffer(vertexBuffer);
				std::vector<uint32_t> indices;
				for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
					aiFace face = mesh->mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++) {
						indices.push_back(face.mIndices[j]);
					}
				}
				std::shared_ptr<IndexBuffer> indexBuffer;
				indexBuffer.reset(IndexBuffer::Create(indices.data(), indices.size()));
				vertexArray->SetIndexBuffer(indexBuffer);
				VAOs.push_back(vertexArray);
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
						if (!ProcessMesh(mesh, scene)) {
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


	class TextureImporter {
	public:

	};

	class MaterialImporter {
	public:
	};

	class SceneImporter {
	public:
	};
}