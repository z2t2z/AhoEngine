#pragma once

#include "Runtime/Function/Renderer/Buffer.h"

namespace Aho {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size, bool dynamicDraw = false);
		OpenGLVertexBuffer(int* vertices, uint32_t size, bool dynamicDraw = false);
		virtual ~OpenGLVertexBuffer();
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void Reset(float* vertices, uint32_t size) override;
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const;
		virtual void Unbind() const;

		virtual uint32_t GetCount() const { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer {
    public:
        OpenGLShaderStorageBuffer(uint32_t size, const void* data = nullptr);
        virtual ~OpenGLShaderStorageBuffer();

		void Bind(uint32_t bindingPoint) const override;
		void Unbind() const override;

		void ClearSSBO(uint32_t size) override;
		void SetData(const void* data, uint32_t size);
		void* GetData() override;

    private:
        uint32_t m_RendererID;
    };


}
