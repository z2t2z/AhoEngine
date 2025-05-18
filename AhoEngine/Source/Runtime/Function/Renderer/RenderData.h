#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "VertexArray.h"
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
		TransformParam* GetTransformParam() { return m_Param; }
		void SetTransformParam(TransformParam* param) { m_Param = param; }
		void Bind(const std::shared_ptr<Shader>& shader, uint32_t texOffset = 0);
		void SetInstanced() { m_Instanced = true; };
		void SetShouldWriteStencil(bool state) { m_WriteStencil = state; };
		bool GetWriteStencil() { return m_WriteStencil; }
		void SetDebug(bool state = true) { m_IsDebug = state; }
		void Unbind();
		void SetEntityID(uint32_t id) { m_EntityID = id; }
		int& GetBoneOffset() { return m_BoneOffset; }
		uint32_t GetEntityID() { return m_EntityID; }
	public:
		bool ShouldBeRendered() { return m_ShouldRender; }
		bool IsDebug() { return m_IsDebug; }
		bool IsInstanced() { return m_Instanced; }
	private:
		bool m_WriteStencil{ false };
		uint32_t m_EntityID{ 0u };
		bool m_IsDebug{ false };
		bool m_Instanced{ false };
		bool m_ShouldRender{ true };
		int m_BoneOffset{ -1 };
	private:
		TransformParam* m_Param{ nullptr };
		std::shared_ptr<VertexArray> m_VAO{ nullptr };
		std::shared_ptr<Material> m_Material{ nullptr };
	};
} // namespace Aho