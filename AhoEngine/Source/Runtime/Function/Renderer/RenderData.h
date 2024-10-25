#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "VertexArrayr.h"
#include "Framebuffer.h"
#include <memory>
#include <unordered_map>

namespace Aho {
	// RenderData is a per mesh basis
	class RenderData {
	public:
		RenderData() = default;
		~RenderData() = default;
		RenderData(const std::shared_ptr<VertexArray>& vao) : m_VAO(vao) {}
		void SetVAOs(const std::shared_ptr<VertexArray>& vao) { m_VAO = vao; }
		std::shared_ptr<VertexArray> GetVAO() { return m_VAO; }
		void SetMaterial(const std::shared_ptr<Material>& mat) { m_Material = mat; }
		std::shared_ptr<Material> GetMaterial() { return m_Material; }
		void SetVirtual() { m_Virtual = true; }
		void SetLine() { m_IsLine = true; }
		TransformParam* GetTransformParam() { return m_Param; }
		void SetTransformParam(TransformParam* param) { m_Param = param; }
		void Bind(const std::shared_ptr<Shader>& shader, uint32_t texOffset = 0);
		void Unbind();
		bool IsLine() { return m_IsLine; }
		bool IsVirtual() { return m_Virtual; }
	private:
		bool m_IsLine{ false };
		bool m_Loaded{ true };
		bool m_Deleted{ false };
		bool m_Rendered{ true };
		bool m_Virtual{ false };
	private:
		TransformParam* m_Param;
		std::shared_ptr<VertexArray> m_VAO{ nullptr };
		std::shared_ptr<Material> m_Material{ nullptr };
	};
} // namespace Aho