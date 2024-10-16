#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho {
	struct Vertex {
		float x, y, z;			// position
		float nx, ny, nz;		// normal
		float tx, ty, tz;		// tangent
		float btx, bty, btz;	// bitangent
		float u, v;				// texture coordinates
		Vertex() { x = y = z = nx = ny = nz = tx = ty = tz = btx = bty = btz = u = v = 0.0f; }
	};

	constexpr int MAX_BONE = 4;
	struct VertexSkeletal {
		float x, y, z;			// position
		float nx, ny, nz;		// normal
		float tx, ty, tz;		// tangent
		float btx, bty, btz;	// bitangent
		float u, v;				// texture coordinates
		int bones[MAX_BONE];
		float weights[MAX_BONE];
		VertexSkeletal() { 
			x = y = z = nx = ny = nz = tx = ty = tz = btx = bty = btz = u = v = 0.0f; 
			memset(bones, -1, sizeof(bones)); 
			memset(weights, 0, sizeof(weights)); 
		}
	};

	struct TransformParam {
		glm::vec3 Translation;
		glm::vec3 Scale;
		glm::vec3 Rotation;
		TransformParam() : Translation(0.0f), Scale(1.0f), Rotation(0.0f) {}
		glm::mat4 GetTransform() const {
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	}; // TODO

	// For loading only
	struct MaterialInfo {
		std::vector<std::pair<TextureType, std::string>> materials;
		MaterialInfo() = default;
		bool HasMaterial() {
			return !materials.empty();
		}
	};

	struct MeshInfo {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal{ false };
		bool hasUVs{ false };
		MaterialInfo materialInfo; // to be removed
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) { }
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {
		}
	};

	struct BoneInfo {
		int id;				// index in finalBoneMatrices
		glm::mat4 offset;	// offset matrix transforms vertex from model space to bone space
	};

	struct SkeletalMeshInfo {
		std::vector<VertexSkeletal> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		std::vector<BoneInfo> boneInfos;
		uint32_t boneCounter{ 0u };
		bool hasNormal{ false };
		bool hasUVs{ false };
		MaterialInfo materialInfo;
		SkeletalMeshInfo(const std::vector<VertexSkeletal>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) {}
		SkeletalMeshInfo(const std::vector<VertexSkeletal>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {}
	};
}