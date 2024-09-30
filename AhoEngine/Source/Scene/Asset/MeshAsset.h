#pragma once

#include "Asset.h"
#include "Core/Renderer/VertexArrayr.h"
#include "Core/Renderer/Shader.h"
#include <memory>

namespace Aho{
	class Material;

	class MeshAsset : public Asset {
	public:
		MeshAsset(const std::string& filepath, std::vector<std::shared_ptr<VertexArray>>&& VAOs, std::vector<std::shared_ptr<Material>>&& Materials)
			: Asset(filepath), m_VAOs(std::move(VAOs)), m_Materials(std::move(Materials)) {
		}

		void SetMaterial(const std::shared_ptr<Shader>& shader) {
			// TODO
		}
		std::vector<std::shared_ptr<VertexArray>>::iterator begin() { return m_VAOs.begin(); }
		std::vector<std::shared_ptr<VertexArray>>::iterator end() { return m_VAOs.end(); }
	private:
		std::vector<std::shared_ptr<VertexArray>> m_VAOs;
		std::vector<std::shared_ptr<Material>> m_Materials;
	};
}