#pragma once

#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Core/Geometry/Mesh.h"

namespace Aho {
	class MeshAsset : public Asset {
	public:
		MeshAsset(const std::string& path, const Mesh& mesh) 
			: Asset(path), m_Mesh(mesh) {}
		Mesh GetMesh() const { return m_Mesh; }
	private:
		Mesh m_Mesh;
	};
}
