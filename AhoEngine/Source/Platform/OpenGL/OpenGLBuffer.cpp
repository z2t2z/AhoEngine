#include "Ahopch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>


namespace Aho {
	// VertexBuffer /////////////////////////////////////////////////////////////
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// IndexBuffer //////////////////////////////////////////////////////////////
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count) {
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// ShaderStorageBuffer //////////////////////////////////////////////////////////////
	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(uint32_t size, const void* data) {
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::Bind(uint32_t bindingPoint) const {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::Unbind() const {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, uint32_t size) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void* OpenGLShaderStorageBuffer::GetData() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		void* data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//if (data != nullptr) {
		//	uint32_t* typedData = static_cast<uint32_t*>(data);
		//	for (size_t i = 0; i < 100; ++i) {
		//		AHO_CORE_TRACE("{}", typedData[i]);
		//	}
		//}


		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return data;
	}
}
