#include "Ahopch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>


namespace Aho {
	// VertexBuffer /////////////////////////////////////////////////////////////
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size, bool dynamicDraw) {
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, dynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(int* vertices, uint32_t size, bool dynamicDraw) {
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, dynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
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

	void OpenGLVertexBuffer::Reset(float* vertices, uint32_t size) {
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices); 
	}

	// IndexBuffer //////////////////////////////////////////////////////////////
	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
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
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void* OpenGLShaderStorageBuffer::GetData() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		void* data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return data;
	}

	void OpenGLShaderStorageBuffer::ClearSSBO(uint32_t size) {
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		//glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	DispatchIndirectBuffer::DispatchIndirectBuffer(uint32_t bufferId) {
		m_BufferId = bufferId;
		Bind();
		struct DispatchIndirectCommand {
			GLuint num_groups_x;
			GLuint num_groups_y;
			GLuint num_groups_z;
		};
		glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(DispatchIndirectCommand), nullptr, GL_DYNAMIC_DRAW);
		Unbind();
	}

	DispatchIndirectBuffer::~DispatchIndirectBuffer() {
		//glDeleteBuffers(1, &m_BufferId);
	}

	void DispatchIndirectBuffer::Bind() const {
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, m_BufferId);
	}

	void DispatchIndirectBuffer::Unbind() const {
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
	}
}
