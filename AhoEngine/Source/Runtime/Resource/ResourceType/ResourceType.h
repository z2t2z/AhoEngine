#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <vector>

#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho {
	enum class LightType {
		None = 0,
		PointLight,
		DirectionalLight,
		RecLight,
		SpotLgiht
	};

	struct Vertex {
		float x, y, z;			// position
		float nx, ny, nz;		// normal
		float tx, ty, tz;		// tangent
		float btx, bty, btz;	// bitangent
		float u, v;				// texture coordinates
		Vertex() { x = y = z = nx = ny = nz = tx = ty = tz = btx = bty = btz = u = v = 0.0f; }
	};

	constexpr int MAX_BONES = 4;
	struct VertexSkeletal {
		float x, y, z;			// position
		float nx, ny, nz;		// normal
		float tx, ty, tz;		// tangent
		float btx, bty, btz;	// bitangent
		float u, v;				// texture coordinates
		float weights[MAX_BONES];
		int bonesID[MAX_BONES];
		VertexSkeletal() { 
			x = y = z = nx = ny = nz = tx = ty = tz = btx = bty = btz = u = v = 0.0f; 
			memset(bonesID, -1, sizeof(bonesID));
			memset(weights, 0, sizeof(weights)); 
		}
	};

	struct TransformParam {
		glm::vec3 Translation;
		glm::vec3 Scale;
		glm::vec3 Rotation; // in degrees
		TransformParam() : Translation(0.0f), Scale(1.0f), Rotation(0.0f) {}
		glm::mat4 GetTransform() {
			glm::mat4 result(1.0f);
			result = glm::rotate(result, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			result = glm::rotate(result, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			result = glm::rotate(result, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			result = glm::translate(glm::mat4(1.0f), Translation) * result * glm::scale(glm::mat4(1.0f), Scale);
			return result;
		}
	}; // TODO

	struct KeyframePosition {
		glm::vec3 attribute{ 0.0f };
		float timeStamp{ 0.0f };
		KeyframePosition() = default;
		KeyframePosition(const glm::vec3& pos, float t) : attribute(pos), timeStamp(t) {}
	};
	struct KeyframeRotation {
		glm::quat attribute;
		float timeStamp{ 0.0f };
		KeyframeRotation() = default;
		KeyframeRotation(const glm::quat& ori, float t) : attribute(ori), timeStamp(t) {}
	};
	struct KeyframeScale {
		glm::vec3 attribute{ 1.0f };
		float timeStamp{ 0.0f };
		KeyframeScale() = default;
		KeyframeScale(const glm::vec3& _scale, float t) : attribute(_scale), timeStamp(t) {}
	};

	// For loading only
	struct MaterialInfo {
		std::vector<std::pair<TextureType, std::string>> materials;
		MaterialInfo() = default;
		bool HasMaterial() { return !materials.empty(); }
	};

	struct MeshInfo {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal;
		bool hasUVs;
		MaterialInfo materialInfo; // to be removed
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) { }
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {
		}
	};

	struct Bone {
		std::string name;
		int id;				// index in finalBoneMatrices
		glm::mat4 offset;	// offset matrix transforms vertex from model space to bone space
		bool hasAnim;
		Bone(int _id, const std::string& _name, const glm::mat4& _offset) : name(_name), id(_id), offset(_offset), hasAnim(false) {}
		Bone() = default;
	};

	// TODO: use a bonePool to store all bones to ensure memory contiguous
	struct BoneNode {
		BoneNode(Bone _bone) : bone(_bone) {}
		~BoneNode() { for (auto child : children) delete child; }
		Bone bone;
		glm::mat4 transform;
		BoneNode* parent{ nullptr };
		std::vector<BoneNode*> children;
	};

	struct SkeletalMeshInfo {
		std::vector<VertexSkeletal> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal;
		bool hasUVs;
		MaterialInfo materialInfo;
		SkeletalMeshInfo(const std::vector<VertexSkeletal>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) {}
		SkeletalMeshInfo(const std::vector<VertexSkeletal>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {}
	};
}