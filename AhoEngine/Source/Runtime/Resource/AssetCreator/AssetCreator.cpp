#include "Ahopch.h"
#include "AssetCreator.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <queue>
#include <cstdlib>

namespace Aho {
	uint32_t AssetCreator::s_MeshCnt = 0;

	std::shared_ptr<StaticMesh> AssetCreator::MeshAssetCreater(const std::string& filePath) {
		auto it = filePath.find_last_of('/\\');
		std::string prefix;
		if (it != std::string::npos) {
			prefix = filePath.substr(0, it) + '/';
		}
		std::string fileName = filePath.substr(it);
		it = fileName.find_last_of('.');
		if (it != std::string::npos) {
			fileName = fileName.substr(0, it);
		}
		it = fileName.find_first_not_of('/\\'); 
		if (it != std::string::npos) {
			fileName = fileName.substr(it);
		}

		if (fileName.empty()) {
			fileName = "Untitled" + std::to_string(s_MeshCnt++);
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
			for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_SHININESS, aiTextureType_HEIGHT, aiTextureType_SPECULAR, aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION }) {
				for (size_t i = 0; i < material->GetTextureCount(type); i++) {
					aiString str;
					AHO_CORE_WARN(std::string(str.data));
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

		return std::make_shared<StaticMesh>(subMesh, fileName);
	}

	std::shared_ptr<SkeletalMesh> AssetCreator::SkeletalMeshAssetCreator(const std::string& filePath) {
		Assimp::Importer importer;
		// TODO: flags can be customized in editor
		// like path settings, absolute or relative
		auto Flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;	// | aiProcess_PreTransformVertices;
		const aiScene* scene = importer.ReadFile(filePath, Flags);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			AHO_CORE_ERROR(importer.GetErrorString());
			return nullptr;
		}
		auto it = filePath.find_last_of('/\\');
		std::string prefix;
		if (it != std::string::npos) {
			prefix = filePath.substr(0, it) + '/';
		}
		std::string fileName = filePath.substr(it);
		it = fileName.find_last_of('.');
		if (it != std::string::npos) {
			fileName = fileName.substr(0, it);
		}
		if (fileName.empty()) {
			fileName = "Untitled" + std::to_string(s_MeshCnt++);
		}

		auto globalInverse = Utils::AssimpMatrixConvertor(scene->mRootNode->mTransformation.Inverse());

		auto RetrieveMaterial = [&](aiMesh* mesh, const aiScene* scene) -> MaterialInfo {
			MaterialInfo info;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			for (const auto& type : { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_SPECULAR, aiTextureType_METALNESS, aiTextureType_AMBIENT_OCCLUSION }) {
				for (size_t i = 0; i < material->GetTextureCount(type); i++) {
					aiString str;
					material->GetTexture(type, i, &str);
					info.materials.emplace_back(Utils::AssimpTextureConvertor(type), prefix + std::string(str.data));
				}
			}
			return info;
		};

		std::vector<std::shared_ptr<SkeletalMeshInfo>> subMesh;
		std::map<std::string, Bone> boneCache;
		uint32_t bonesCnt = 0;
		auto RetrieveBonesInfo = [&](std::vector<VertexSkeletal>& vertices, aiMesh* mesh, const aiScene* scene) -> void {
			for (uint32_t i = 0; i < mesh->mNumBones; i++) {
				std::string boneName(mesh->mBones[i]->mName.C_Str());
				if (!boneCache.contains(boneName)) {
					Bone bone(bonesCnt++, boneName, Utils::AssimpMatrixConvertor(mesh->mBones[i]->mOffsetMatrix));
					boneCache[boneName] = bone;
				}
				Bone& bone = boneCache[boneName];
				//int id = node->bone.id;
				int id = bone.id;
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
			RetrieveBonesInfo(vertices, mesh, scene);

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

		uint32_t actualBoneCnt = 0;
		std::map<std::string, BoneNode*> boneNodeCache;
		auto BuildHierarchy = [&](auto self, const aiNode* node, BoneNode* parent) -> BoneNode* {
			std::string boneName = node->mName.C_Str();
			BoneNode* boneNode = (boneCache.contains(boneName) ? new BoneNode(boneCache[boneName]) : new BoneNode(Bone(bonesCnt++, boneName, glm::mat4(1.0f))));
			boneNodeCache[boneName] = boneNode;
			BoneNode* currNode = boneNodeCache[boneName];
			currNode->transform = Utils::AssimpMatrixConvertor(node->mTransformation);
			currNode->transformParam = new TransformParam(glm::mat4(1.0f));
			currNode->parent = parent;
			currNode->hasInfluence = boneCache.contains(boneName);
			for (uint32_t i = 0; i < node->mNumChildren; i++) {
				auto res = self(self, node->mChildren[i], currNode);
				if (res) {
					currNode->children.push_back(res);
				}
			}
			if (currNode->hasInfluence) {
				AHO_CORE_WARN(boneName);
			}
			actualBoneCnt += currNode->hasInfluence;
			return currNode;
		};

		BoneNode* root{ nullptr };
		std::vector<BoneNode*> temp;
		auto RemoveExtraBones = [&](auto self, const aiNode* node, BoneNode* parent, glm::mat4 globalMatrix4) -> void {
			std::string name = node->mName.C_Str();
			glm::mat4 localMatrix4 = Utils::AssimpMatrixConvertor(node->mTransformation);
			globalMatrix4 = globalMatrix4 * localMatrix4;
			if (boneCache.contains(name)) {
				BoneNode* bone = new BoneNode(boneCache.at(name));
				boneNodeCache[name] = bone;
				temp.push_back(bone);
				bone->transformParam = new TransformParam(glm::mat4(1.0f));
				bone->hasInfluence = true;
				bone->transform = globalMatrix4;
				if (!root) {
					root = bone;
				}
				else {
					AHO_CORE_ASSERT(parent);
					bone->parent = parent;
					bone->parent->children.push_back(bone);
				}
				globalMatrix4 = glm::mat4(1.0f);
				parent = bone;
			}
			for (uint32_t i = 0; i < node->mNumChildren; i++) {
				self(self, node->mChildren[i], parent, globalMatrix4);
			}
		};

		ProcessNode(scene->mRootNode, scene);
		//BoneNode* Root = BuildHierarchy(BuildHierarchy, scene->mRootNode, nullptr);
		RemoveExtraBones(RemoveExtraBones, scene->mRootNode, nullptr, glm::mat4(1.0f));
		AHO_CORE_ASSERT(root);
		return std::make_shared<SkeletalMesh>(subMesh, boneNodeCache, root, fileName, boneNodeCache.size());
	}

	std::shared_ptr<MaterialAsset> AssetCreator::MaterialAssetCreator(const std::string& filePath) {
		return std::make_shared<MaterialAsset>();
	}
	std::shared_ptr<AnimationAsset> AssetCreator::AnimationAssetCreator(const std::string& filePath, const std::shared_ptr<SkeletalMesh>& mesh) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
			AHO_CORE_ERROR(importer.GetErrorString());
			return nullptr;
		}

		auto globalInverse = Utils::AssimpMatrixConvertor(scene->mRootNode->mTransformation.Inverse());
		auto& boneCache = mesh->GetBoneCache();
		size_t boneCnt = mesh->GetBoneCnt();
		std::vector<std::vector<KeyframePosition>> Positions(boneCnt); 
		std::vector<std::vector<KeyframeRotation>> Rotations(boneCnt);
		std::vector<std::vector<KeyframeScale>> Scales(boneCnt);
		auto LoadKeyframesData = [&](const aiAnimation* animation) -> void {
			for (uint32_t i = 0; i < animation->mNumChannels; i++) {
				auto channel = animation->mChannels[i];
				std::string name = channel->mNodeName.data;
				//AHO_CORE_WARN(name);
				if (!boneCache.contains(name)) {
					AHO_CORE_ERROR("Does not have bone {} when loading", name);
					continue;
				}
				Bone& bone = boneCache.at(name)->bone;
				// Filling animation data
				if (Positions.size() < bone.id) {
					Positions.resize(bone.id);
				}
				for (size_t i = 0; i < channel->mNumPositionKeys; i++) {
					Positions[bone.id].emplace_back(Utils::AssimpVec3Convertor(channel->mPositionKeys[i].mValue), channel->mPositionKeys[i].mTime);
				}
				if (Rotations.size() < bone.id) {
					Rotations.resize(bone.id);
				}
				for (size_t i = 0; i < channel->mNumRotationKeys; i++) {
					Rotations[bone.id].emplace_back(glm::normalize(Utils::AssimpQuatConvertor(channel->mRotationKeys[i].mValue)), channel->mRotationKeys[i].mTime);
				}
				if (Scales.size() < bone.id) {
					Scales.resize(bone.id);
				}
				for (size_t i = 0; i < channel->mNumScalingKeys; i++) {
					Scales[bone.id].emplace_back(Utils::AssimpVec3Convertor(channel->mScalingKeys[i].mValue), channel->mScalingKeys[i].mTime);
				}
			}
		};

		auto animation = scene->mAnimations[0];
		//aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		//globalTransformation = globalTransformation.Inverse();
		float duration = animation->mDuration;
		int TicksPerSecond = animation->mTicksPerSecond;
		LoadKeyframesData(animation);
		auto Sort = [](auto& vec) -> void {
			std::sort(vec.begin(), vec.end(), [&](auto& lhs, auto& rhs) {
				return lhs.timeStamp < rhs.timeStamp;
			});
		};
		Positions.shrink_to_fit();
		for (auto& vec : Positions) {
			Sort(vec);
		}
		Rotations.shrink_to_fit();
		for (auto& vec : Rotations) {
			Sort(vec);
		}
		Scales.shrink_to_fit();
		for (auto& vec : Scales) {
			Sort(vec);
		}
		return std::make_shared<AnimationAsset>(Positions, Rotations, Scales, duration, TicksPerSecond, globalInverse);
	}
} // namespace Aho