#pragma once

#include "Asset.h"
#include "Runtime/Function/Renderer/Material.h"
#include "Runtime/Function/Renderer/Texture.h"
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

	class TextureImporter {
	public:
		static std::shared_ptr<Asset>ImportTexture(const std::filesystem::path& filePath) {
			return std::make_shared<TextureAsset>(filePath.string(), Texture2D::Create(filePath.string()));
		}
	};

	class MeshImporter {
		using VertexArrayList = std::vector<std::shared_ptr<VertexArray>>;
		using MeterialList = std::vector<std::shared_ptr<MaterialAsset>>;
	public:
		static std::shared_ptr<Asset>ImportMesh(const std::filesystem::path& filePath) {
			VertexArrayList VAOs;
			MeterialList Materials;
			if (!LoadModel(filePath, VAOs, Materials)) {
				AHO_CORE_ERROR("Failed to load mesh");
				return nullptr;
			}
			std::shared_ptr<MeshAsset> meshAsset = std::make_shared<MeshAsset>(filePath.string(), std::move(VAOs), std::move(Materials));
			if (!meshAsset) {
				AHO_CORE_ERROR("Failed to create mesh asset");
				return nullptr;
			}
			meshAsset->SetType(AssetType::Mesh);
			return meshAsset;
		}
	private:
		// BIG TODO!!!!!
		static bool LoadModel(const std::filesystem::path& filePath, VertexArrayList& VAOs, MeterialList& Materials) {
			Assimp::Importer importer;
			auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
			const aiScene* scene = importer.ReadFile(filePath.string(), Flags);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				AHO_CORE_ERROR(importer.GetErrorString());
				return false;
			}
			std::string directory = filePath.string().substr(0, filePath.string().find_last_of('/'));
			std::array<aiTextureType, 4> TextureTypes = { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_SPECULAR, aiTextureType_HEIGHT };
			std::unordered_map<std::string, std::shared_ptr<TextureAsset>> Loaded;
			auto ProcessMaterials = [&](aiMaterial* aiMat) -> bool {
				std::shared_ptr<MaterialAsset> matAsset = std::make_shared<MaterialAsset>(directory, std::make_shared<Material>());
				for (const auto& type : TextureTypes) {
					for (size_t i = 0; i < aiMat->GetTextureCount(type); i++) {
						aiString str;
						aiMat->GetTexture(type, i, &str);
						std::string cppstr = std::string(str.C_Str());
						if (Loaded.contains(cppstr)) {
							//Materials.push_back(Loaded[cppstr]);
							continue;
						}
						auto textureAsset = std::make_shared<TextureAsset>(cppstr, Texture2D::Create(cppstr));
						Loaded[cppstr] = textureAsset;
						textureAsset->GetTexture()->SetTextureType(i == 0 ? TextureType::Diffuse : TextureType::Normal);
						matAsset->GetMaterial()->AddTexture(textureAsset->GetTexture());
						Materials.push_back(matAsset);
					}
				}
				return true;
			};

			auto ProcessMesh = [&](aiMesh* mesh, const aiScene* scene) -> bool {
				int normal_cnt = 0;
				int texcoords_cnt = 0;
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
						//normal_cnt++;
						vertices.push_back(mesh->mNormals[i].x);
						vertices.push_back(mesh->mNormals[i].y);
						vertices.push_back(mesh->mNormals[i].z);
					}
					else {
						vertices.push_back(0.0f);
						vertices.push_back(1.0f);
						vertices.push_back(0.0f);
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
					else {
						for (int i = 0; i < 8; i++) {
							vertices.push_back(0.0f);
						}
					}
				}
				//AHO_CORE_ASSERT(normal_cnt == texcoords_cnt);
				//if (normal_cnt != 0) {
				//	AHO_CORE_ASSERT(vertics.size() == normal_cnt);
				//}
				//AHO_CORE_ASSERT()
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
				if (!ProcessMaterials(scene->mMaterials[mesh->mMaterialIndex])) {
					return false;
				}
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

	class MaterialImporter {
	public:
	};

	class SceneImporter {
	public:
	};
}