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
	};

	struct MeshInfo {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		bool hasNormal{ false };
		MaterialInfo materialInfo;
		MeshInfo(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, bool _hasNormal, const MaterialInfo& info)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), hasNormal(_hasNormal), materialInfo(info) { }
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
		uint32_t size() { return m_SubMesh.size(); }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};
}