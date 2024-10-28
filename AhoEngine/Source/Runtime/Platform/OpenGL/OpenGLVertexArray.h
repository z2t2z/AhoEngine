#pragma once

#include "Runtime/Function/Renderer/VertexArrayr.h"
#include "Runtime/Function/Renderer/Buffer.h"

namespace Aho {
	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray(bool dynamicDraw = false);
		virtual ~OpenGLVertexArray();

		virtual void Init(const std::shared_ptr<LineInfo>& lineInfo) override;
		virtual void Init(const std::shared_ptr<MeshInfo>& meshInfo) override;
		virtual void Init(const std::shared_ptr<SkeletalMeshInfo>& meshInfo) override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual uint32_t GetInstanceAmount() const override { return m_InstanceAmount; }
		virtual void SetInstancedAmount(uint32_t amount) override { m_InstanceAmount = amount; }
		virtual void SetInstancedTransform(const std::vector<glm::mat4>& transform, bool dynamicDraw) override;
		virtual void UpdateInstancedTransform(const std::vector<glm::mat4>& transform) override;
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
		//virtual std::vector<std::shared_ptr<VertexBuffer>> GetVertexBuffer() { return m_VertexBuffers; }

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		bool m_Dynamic{ false };
		uint32_t m_Offset; //  glVertexAttribPointer(m_Offset, ...)
		uint32_t m_InstanceAmount;
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
} // namespace Aho