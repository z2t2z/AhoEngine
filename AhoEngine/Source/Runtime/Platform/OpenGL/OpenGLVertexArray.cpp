#include "Ahopch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Aho {
	namespace Utils {
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
	}

	OpenGLVertexArray::OpenGLVertexArray(bool dynamicDraw) 
		: m_Dynamic(dynamicDraw), m_Offset{ 0U } {
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Init(const std::shared_ptr<LineInfo>& lineInfo) {
		auto& vertices = lineInfo->vertices;
		auto& indices = lineInfo->indices;
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float), m_Dynamic));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" }
		};
		vertexBuffer->SetLayout(layout);
		AddVertexBuffer(vertexBuffer);
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices.data(), indices.size()));
		SetIndexBuffer(indexBuffer);
	}


	void OpenGLVertexArray::Init(const std::shared_ptr<MeshInfo>& meshInfo) {
		std::vector<float> vertices;
		vertices.reserve(meshInfo->vertexBuffer.size() * 14u);
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

	void OpenGLVertexArray::Init(const std::shared_ptr<SkeletalMeshInfo>& meshInfo) {
		std::vector<float> vertices;
		std::vector<int> verticesI;
		//vertices.reserve();
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
			for (int i = 0; i < MAX_BONES; i++) {
				vertices.push_back(vertex.weights[i]);
			}
			for (int i = 0; i < MAX_BONES; i++) {
				verticesI.push_back(vertex.bonesID[i]);
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
		layout.Push({ ShaderDataType::Float4, "a_BoneWeights" });
		vertexBuffer->SetLayout(layout);
		AddVertexBuffer(vertexBuffer);

		if (!verticesI.empty()) {
			std::shared_ptr<VertexBuffer> vertexBufferI;
			vertexBufferI.reset(VertexBuffer::Create(verticesI.data(), verticesI.size() * sizeof(int)));
			BufferLayout layoutI = {
				{ ShaderDataType::Int4, "a_BoneID" }
			};
			vertexBufferI->SetLayout(layoutI);
			AddVertexBuffer(vertexBufferI);
		}

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

	// TODO: hardcode the pointers!!
	void OpenGLVertexArray::SetInstancedTransform(const std::vector<glm::mat4>& transform, bool dynamicDraw) {
		std::shared_ptr<VertexBuffer> vertexBuffer;
		m_InstanceAmount = transform.size();
		vertexBuffer.reset(VertexBuffer::Create((float*)&transform[0], m_InstanceAmount * sizeof(glm::mat4), dynamicDraw));
		
		//glBindVertexArray(m_RendererID);
		Bind();
		vertexBuffer->Bind();
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));

		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);

		m_VertexBuffers.push_back(vertexBuffer);
		vertexBuffer->Unbind();
		Unbind();
	}

	void OpenGLVertexArray::UpdateInstancedTransform(const std::vector<glm::mat4>& transform) {
		// TODO;
		Bind();
		m_VertexBuffers.back()->Bind();
		m_VertexBuffers.back()->Reset((float*)&transform[0], m_InstanceAmount * sizeof(glm::mat4));
		m_VertexBuffers.back()->Unbind();
		Unbind();
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
		AHO_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			if (element.Type == ShaderDataType::Int4) {
				glVertexAttribIPointer(m_Offset,
					element.GetComponentCount(),
					Utils::ShaderDataTypeToOpenGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)element.Offset);
			}
			else {
				glVertexAttribPointer(m_Offset,
					element.GetComponentCount(),
					Utils::ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
			}
			glEnableVertexAttribArray(m_Offset);
			m_Offset++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}