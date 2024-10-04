#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include <vector>

namespace Aho {
	struct Vertex {
		float x, y, z;			// position
		float nx, ny, nz;		// normal
		float tx, ty, tz;		// tangent
		float btx, bty, btz;	// bitangent
		float u, v;				// texture coordinates
	};

	struct MaterialInfo {
		bool hasMaterial{ false };
		std::string Albedo;
		std::string Normal;
		std::string Specular;
		std::string Metallic;
		std::string Roughness;

		MaterialInfo() = default;
		template<typename... Args>
		MaterialInfo(bool has, Args&&... args)
			: hasMaterial(has), Albedo(std::forward<Args>(args)...) {
			if constexpr (sizeof...(args) > 1) Normal = std::forward<Args>(args + 1);
			if constexpr (sizeof...(args) > 2) Specular = std::forward<Args>(args + 2);
			if constexpr (sizeof...(args) > 3) Metallic = std::forward<Args>(args + 3);
			if constexpr (sizeof...(args) > 4) Roughness = std::forward<Args>(args + 4);
		}

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
		Asset() = default;
		Asset(const std::string& path) {}
		
		virtual ~Asset() = default;
		//virtual bool Load() = 0;
		//virtual bool Save() = 0;
		bool IsLoaded() { return m_IsLoaded; }
		UUID GetUUID() { return m_UUID; }
	private:
		UUID m_UUID;
		std::string m_Path;
		bool m_IsLoaded{ false };
	};

	class StaticMesh : Asset {
	public:
		StaticMesh() = default;
		StaticMesh(const std::string& path) {}
		StaticMesh(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh) : m_SubMesh(SubMesh) {}

		std::vector<std::shared_ptr<MeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<MeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	class MaterialAsset : Asset {
	public:
		MaterialAsset() = default;
		MaterialAsset(const std::string& path) {}
		MaterialAsset(const MaterialInfo& materialInfo) : m_MaterialInfo(materialInfo) {}
		MaterialInfo GetMaterialInfo() { return m_MaterialInfo; }
	private:
		MaterialInfo m_MaterialInfo;
	};
}