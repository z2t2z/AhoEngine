#pragma once

#include "Runtime/Resource/UUID/UUID.h"
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
	};

	struct TransformPara {
		glm::vec3 Translation;
		glm::vec3 Scale;
		glm::vec3 Rotation;
		TransformPara() : Translation(0.0f), Scale(1.0f), Rotation(0.0f) {}
		glm::mat4 GetTransform() const {
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	}; // BIG TODO: Idk where this shoule be

	struct MaterialInfo {
		std::vector<std::string> Albedo;
		std::vector<std::string> Normal;
		MaterialInfo() = default;
		//MaterialInfo(const MaterialInfo& m); // ?
		//MaterialInfo(MaterialInfo&& other) noexcept : Albedo(other.Albedo), Normal(other.Normal) {}
		bool HasMaterial() {
			return !Albedo.empty() || !Normal.empty();
		}
	};

	struct MeshInfo {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal{ false };
		bool hasUVs{ false };
		MaterialInfo materialInfo;
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, bool _hasUV, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), hasUVs(_hasUV), materialInfo(info) { }
	};

	/*
		Asset can be created through:
		1. Constructed using the data loaded from DCC
		2. Reading .asset file from the disk
	*/

	class Asset {
	public:
		virtual ~Asset() = default;
		virtual bool Load() = 0;
		//virtual bool Save() = 0;
		bool IsLoaded() { return m_IsLoaded; }
		UUID GetUUID() { return m_UUID; }
	protected:
		UUID m_UUID;
		std::string m_Path;
		bool m_IsLoaded{ false };
	};

	class StaticMesh : public Asset {
	public:
		StaticMesh() = default;
		StaticMesh(const std::string& path) {}
		StaticMesh(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh) : m_SubMesh(SubMesh) {}
		virtual bool Load() override { return false; }
		std::vector<std::shared_ptr<MeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<MeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	//class ShaderAsset : Asset {
	//public:
	//	ShaderAsset() = default;
	//private:
	//	std::shared_ptr<Shader> m_Shader;
	//};

	class MaterialAsset : public Asset {
	public:
		MaterialAsset() = default;
		MaterialAsset(const std::string& path) {}
		MaterialAsset(const MaterialInfo& materialInfo) : m_MaterialInfo(materialInfo) {}
		virtual bool Load() override {}
		MaterialInfo GetMaterialInfo() { return m_MaterialInfo; }
	private:
		MaterialInfo m_MaterialInfo;
	};
}