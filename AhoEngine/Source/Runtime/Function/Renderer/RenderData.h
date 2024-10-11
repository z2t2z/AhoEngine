#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "VertexArrayr.h"
#include <memory>
#include <unordered_map>

namespace Aho {
	class RenderData {
	public:
		RenderData() = default;
		~RenderData() = default;
		void SetVAOs(const std::shared_ptr<VertexArray>& vao) { m_VAO = vao; }
		std::shared_ptr<VertexArray> GetVAO()					{ return m_VAO; }
		void SetMaterial(const std::shared_ptr<Material>& mat) { m_Material = mat; }
		std::shared_ptr<Material> GetMaterial() { return m_Material; }
		void Bind();
		void Unbind();
	private:
		std::shared_ptr<VertexArray> m_VAO{ nullptr };
		std::shared_ptr<Material> m_Material{ nullptr };	// multiple materials?
	// TODO:
		//std::shared_ptr<Shader> m_Shader{ nullptr };		// TODO: think about whether shader should be here or inside material
		//void SetShader(const std::shared_ptr<Shader>& shader)	{ m_Shader = shader; }
	};
} // namespace Aho