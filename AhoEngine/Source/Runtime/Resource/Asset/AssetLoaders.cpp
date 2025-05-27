#include "Ahopch.h"
#include "AssetLoaders.h"
#include "AssetLoadOptions.h"

#include "Runtime/Core/Geometry/Mesh.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <queue>
#include <cstdlib>

namespace Aho {
	// Some helper functions
	namespace {
		TextureUsage AssimpTextureToTextureUsageConvertor(aiTextureType type) {
			switch (type) {
			case (aiTextureType_DIFFUSE):
				return TextureUsage::BaseColor;
			case (aiTextureType_BASE_COLOR):
				return TextureUsage::BaseColor;
			case (aiTextureType_NORMALS):
				return TextureUsage::Normal;
			case (aiTextureType_NORMAL_CAMERA):
				return TextureUsage::Normal;
			case (aiTextureType_EMISSION_COLOR):
				return TextureUsage::Emissive;
			case (aiTextureType_EMISSIVE):
				return TextureUsage::Emissive;
			case (aiTextureType_SPECULAR):
				AHO_CORE_ASSERT(false, "Not yet supported");
				return TextureUsage::Specular;
			case (aiTextureType_METALNESS):
				return TextureUsage::Metallic;
			case (aiTextureType_AMBIENT_OCCLUSION):
				return TextureUsage::AO;
			case (aiTextureType_DIFFUSE_ROUGHNESS):
				return TextureUsage::Roughness;
			case (aiTextureType_SHININESS):
				return TextureUsage::Roughness;
			default:
				AHO_CORE_ASSERT(false, "Texture type not yet supported");
			}
		}

		// Convert assimp matrix to glm matrx
		glm::mat4 AssimpMatrixConvertor(const aiMatrix4x4& from) {
			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		glm::vec3 AssimpVec3Convertor(const aiVector3D& vec) {
			return glm::vec3(vec.x, vec.y, vec.z);
		}

		glm::quat AssimpQuatConvertor(const aiQuaternion& pOrientation) {
			return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
		}
	}

	bool AssetLoader::MeshLoader(const MeshOptions& options, std::vector<Mesh>& meshes, std::vector<MaterialPaths>& mats) {
		std::string path		= options.path;
		glm::mat4 preTransform	= options.PreTransform;

		Assimp::Importer importer;
		// TODO: flags can be customized in load options
		auto Flags	= options.LoadFlags;
		Flags		= aiProcess_Triangulate
											| aiProcess_FixInfacingNormals
											| aiProcess_FlipUVs
											| aiProcess_SplitLargeMeshes
											| aiProcess_CalcTangentSpace
											| aiProcess_OptimizeMeshes
											| aiProcess_OptimizeGraph
											| aiProcessPreset_TargetRealtime_Fast
											;

		const aiScene* scene = importer.ReadFile(path, Flags);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			AHO_CORE_ERROR("Errors in loading mesh from: {}\nError Message: {}", path, importer.GetErrorString());
			return false;
		}

		auto ProcessMaterial =
			[&mats](aiMesh* mesh, const aiScene* scene) -> void {
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<std::pair<TextureUsage, std::string>> UsagePaths;
				for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_EMISSION_COLOR, aiTextureType_EMISSIVE, aiTextureType_SHININESS, /*aiTextureType_SPECULAR,*/ aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION }) {
					for (size_t i = 0; i < material->GetTextureCount(type); i++) {
						aiString path;
						material->GetTexture(type, i, &path);
						TextureUsage tu = AssimpTextureToTextureUsageConvertor(type);
						UsagePaths.emplace_back(tu, path.data);
					}
				}
				mats.emplace_back(UsagePaths);
			};

		uint32_t vertexCnt = 0u;
		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(preTransform)));
		auto ProcessSubMesh = 
			[&preTransform, &vertexCnt, &meshes, &normalMat](aiMesh* mesh, const aiScene* scene) -> void {
				std::vector<Vertex> vertices;
				vertices.reserve(mesh->mNumVertices);

				bool hasNormal = mesh->HasNormals();
				bool hasUVs = mesh->HasTextureCoords(0);
				vertexCnt += mesh->mNumVertices;

				for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
					Vertex vertex;
					vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
					vertex.position = glm::vec3(preTransform * glm::vec4(vertex.position, 1.0f));

					if (hasNormal) {
						vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
						vertex.normal = normalMat * vertex.normal;
					}
					// Texture coords, tangent and bitangent
					if (hasUVs) {
						vertex.u = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y).x;
						vertex.v = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y).y;

						vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
					}
					vertices.push_back(vertex);
				}
				std::vector<uint32_t> indices;
				indices.reserve(mesh->mNumVertices);
				for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
					aiFace face = mesh->mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++) {
						indices.push_back(face.mIndices[j]);
					}
				}
				AHO_CORE_TRACE("{}", mesh->mName.data);
				meshes.emplace_back(vertices, indices, mesh->mName.data, hasNormal, hasUVs);
			};

		auto ProcessNode = 
			[&](aiNode* node, const aiScene* scene) -> void {
				std::queue<aiNode*> q;
				q.push(node);
				while (!q.empty()) {
					aiNode* cur_node = q.front();
					q.pop();
					for (uint32_t i = 0; i < cur_node->mNumMeshes; i++) {
						//scene->mName; TODO: Figure out this
						aiMesh* mesh = scene->mMeshes[cur_node->mMeshes[i]];
						ProcessSubMesh(mesh, scene);
						ProcessMaterial(mesh, scene);
					}
					for (uint32_t i = 0; i < cur_node->mNumChildren; i++) {
						q.push(cur_node->mChildren[i]);
					}
				}
			};

		ProcessNode(scene->mRootNode, scene);

		return true;
	}

}