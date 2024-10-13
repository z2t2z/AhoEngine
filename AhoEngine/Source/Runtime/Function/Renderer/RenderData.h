#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "VertexArrayr.h"
#include <memory>
#include <unordered_map>

namespace Aho {
	// RenderData is a per mesh basis
	class RenderData {
	public:
		RenderData() = default;
		~RenderData() { delete m_Para; }
		void SetVAOs(const std::shared_ptr<VertexArray>& vao) { m_VAO = vao; }
		std::shared_ptr<VertexArray> GetVAO() { return m_VAO; }
		void SetMaterial(const std::shared_ptr<Material>& mat) { m_Material = mat; }
		std::shared_ptr<Material> GetMaterial() { return m_Material; }
		void SetTransform(TransformPara* t) { m_Para = t; }
		void ShouldBindMaterial(bool state) { m_BindMaterial = state; }
		void Bind(const std::shared_ptr<Shader>& shader);
		void Unbind();
	private:
		bool m_Rendered{ true };
		bool m_BindMaterial{ true };
	private:
		TransformPara* m_Para{ nullptr };
		std::shared_ptr<VertexArray> m_VAO{ nullptr };
		std::shared_ptr<Material> m_Material{ nullptr };	// multiple materials?
	};

	//void SetShader(const std::shared_ptr<Shader>& shader)	{ m_Shader = shader; }
	//std::shared_ptr<Shader> m_Shader{ nullptr };
} // namespace Aho