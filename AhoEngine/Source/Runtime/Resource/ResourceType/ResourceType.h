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

	struct RawMesh {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
	};

	/* 
		Definition of the base asset class here.
		We should not define asset realted to renderables, scenes, etc. in Resource layer.
		Assets should be mantained as a DAG(or not? Since we need to load/unload in different levels in runtime).
		In Function layer we will define renderables which consists of some base assets here,like StaticMesh and Image
	*/ 

	class Asset {
	public:
		Asset(const UUID& uuid, const std::string& path)
			: m_UUID(uuid), m_Path(path), m_IsLoaded(true) {}
		virtual ~Asset() = default;
		bool IsLoaded() { return m_IsLoaded; }
		UUID GetUUID() { return m_UUID; }
	private:
		UUID m_UUID;
		std::string m_Path;
		bool m_IsLoaded;
	};

	class StaticMesh : Asset {
	public:
		StaticMesh(const UUID& uuid, const std::string& path, std::shared_ptr<RawMesh> mesh)
			: Asset(uuid, path), m_Mesh(mesh) {}
	private:
		std::shared_ptr<RawMesh> m_Mesh;
	};
}