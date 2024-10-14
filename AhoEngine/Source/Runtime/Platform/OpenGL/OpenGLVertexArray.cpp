#include "Ahopch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Aho {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
		switch (type) {
			case Aho::ShaderDataType::Float:    return GL_FLOAT;
			case Aho::ShaderDataType::Float2:   return GL_FLOAT;
			case Aho::ShaderDataType::Float3:   return GL_FLOAT;
			case Aho::ShaderDataType::Float4:   return GL_FLOAT;
			case Aho::ShaderDataType::Mat3:     return GL_FLOAT;
			case Aho::ShaderDataType::Mat4:     return GL_FLOAT;
			case Aho::ShaderDataType::Int:      return GL_INT;
			case Aho::ShaderDataType::Int2:     return GL_INT;
			case Aho::ShaderDataType::Int3:     return GL_INT;
			case Aho::ShaderDataType::Int4:     return GL_INT;
			case Aho::ShaderDataType::Bool:     return GL_BOOL;
		}

		AHO_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray() {
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Init(const std::shared_ptr<MeshInfo>& meshInfo) {
		std::vector<float> vertices;
		//vertices.reserve(meshInfo->vertexBuffer.size() * 14u);
		for (const auto& vertex : meshInfo->vertexBuffer) {
			vertices.push_back(vertex.x);
			vertices.push_back(vertex.y);
			vertices.push_back(vertex.z);
			if (meshInfo->hasNormal) {
				vertices.push_back(vertex.nx);
				vertices.push_back(vertex.ny);
				vertices.push_back(vertex.nz);
			}
			if (meshInfo->hasUVs) {
				vertices.push_back(vertex.u);
				vertices.push_back(vertex.v);
				// TODO;
				vertices.push_back(vertex.tx);
				vertices.push_back(vertex.ty);
				vertices.push_back(vertex.tz);
				vertices.push_back(vertex.btx);
				vertices.push_back(vertex.bty);
				vertices.push_back(vertex.btz);
			}
		}
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float)));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" }
		};
		if (meshInfo->hasNormal) {
			layout.Push({ ShaderDataType::Float3, "a_Normal" });
		}
		if (meshInfo->hasUVs) {
			layout.Push({ ShaderDataType::Float2, "a_TexCoords" });
			// TODO;
			layout.Push({ ShaderDataType::Float3, "a_Tangent" });
			layout.Push({ ShaderDataType::Float3, "a_Bitangent" });
		}
		vertexBuffer->SetLayout(layout);
		AddVertexBuffer(vertexBuffer);
		
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(meshInfo->indexBuffer.data(), meshInfo->indexBuffer.size()));
		SetIndexBuffer(indexBuffer);
	}

	void OpenGLVertexArray::Bind() const {
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const {
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
		AHO_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}