#pragma once

#include "Runtime/Core/Math/Math.h"
#include "Runtime/Core/Geometry/Vertex.h"
#include <vector>

namespace Aho {
	struct TransformParam {
		glm::vec3 Translation{ 0.0f };
		glm::vec3 Scale{ 1.0f };
		glm::vec3 Rotation{ 0.0f };
		glm::quat Orientation;
		TransformParam(const glm::mat4& transform) {
			glm::vec3 Skew;
			glm::vec4 Perspective;
			glm::decompose(transform, Scale, Orientation, Translation, Skew, Perspective);
			Rotation = glm::degrees(QuaternionToEuler(Orientation));
		}
		TransformParam() : Translation(0.0f), Scale(1.0f), Rotation(0.0f) {}
		glm::mat4 GetTransform() {
			Orientation = EulerToQuaternion(Rotation.x, Rotation.y, Rotation.z);
			return GetTransformQuat();
		}
		glm::mat4 GetTransformQuat() {
			return glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(Orientation) * glm::scale(glm::mat4(1.0f), Scale);
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
	enum class TexType;
	struct MaterialInfo {
		std::vector<std::pair<TexType, std::string>> materials;
		MaterialInfo() = default;
		bool HasMaterial() { return !materials.empty(); }
	};

	struct LineInfo {
		std::vector<float> vertices;
		std::vector<uint32_t> indices;
		LineInfo(const std::vector<float>& _vertices, const std::vector<uint32_t>& _indices) : vertices(_vertices), indices(_indices) {}
	};

	struct MeshInfo {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal;
		bool hasUVs;
		MaterialInfo materialInfo; // to be removed
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) {
		}
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {
		}
	};

	struct Bone {
		std::string name;
		int id;				// index in finalBoneMatrices
		glm::mat4 offset;	// offset matrix transforms vertex from model space to bone space
		Bone(int _id, const std::string& _name, const glm::mat4& _offset) : name(_name), id(_id), offset(_offset) {}
		Bone() = default;
	};

	// TODO: use a bonePool to store all bones to ensure memory contiguous
	struct BoneNode {
		BoneNode(Bone _bone) : bone(_bone) {}
		~BoneNode() { for (auto child : children) delete child; }
		Bone bone;
		bool hasInfluence{ false };
		glm::mat4 transform;
		glm::mat4 global;
		TransformParam* transformParam{ nullptr };
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
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) {
		}
		SkeletalMeshInfo(const std::vector<VertexSkeletal>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV) {
		}
	};
}