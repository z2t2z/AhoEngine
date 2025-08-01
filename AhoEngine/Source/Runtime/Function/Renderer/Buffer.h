#pragma once

#include <cstdint>
#include "Runtime/Core/Core.h"

namespace Aho {
	enum class ShaderDataType {
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool
	};

	// 1 Float 4 bytes
	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:    return 4u;
			case ShaderDataType::Float2:   return 4u * 2;
			case ShaderDataType::Float3:   return 4u * 3;
			case ShaderDataType::Float4:   return 4u * 4;
			case ShaderDataType::Mat3:     return 4u * 3 * 3;
			case ShaderDataType::Mat4:     return 4u * 4 * 4;
			case ShaderDataType::Int:      return 4u;
			case ShaderDataType::Int2:     return 4u * 2;
			case ShaderDataType::Int3:     return 4u * 3;
			case ShaderDataType::Int4:     return 4u * 4;
			case ShaderDataType::Bool:     return 1u;
		}
		AHO_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement {
		std::string Name{ 0 };
		ShaderDataType Type{ ShaderDataType::Bool };
		uint32_t Size{ 0 };
		uint32_t Offset{ 0 };
		bool Normalized{ false };
		BufferElement() {}
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) : 
			Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}
		uint32_t GetComponentCount() const {
			switch (Type) {
				case ShaderDataType::Float:   return 1;
				case ShaderDataType::Float2:  return 2;
				case ShaderDataType::Float3:  return 3;
				case ShaderDataType::Float4:  return 4;
				case ShaderDataType::Mat3:    return 3 * 3;
				case ShaderDataType::Mat4:    return 4 * 4;
				case ShaderDataType::Int:     return 1;
				case ShaderDataType::Int2:    return 2;
				case ShaderDataType::Int3:    return 3;
				case ShaderDataType::Int4:    return 4;
				case ShaderDataType::Bool:    return 1;
			}
			AHO_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) {
			CalculateOffsetsAndStride();
		}
		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		void Push(const BufferElement& e) { m_Elements.push_back(e); CalculateOffsetsAndStride(); }
		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride() {
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;
		static IndexBuffer* Create(const uint32_t* indices, uint32_t size);
	};

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual void Reset(float* vertices, uint32_t size) = 0;
		static VertexBuffer* Create(float* vertices, uint32_t size, bool dynamicDraw = false);
		static VertexBuffer* Create(int* vertices, uint32_t size, bool dynamicDraw = false);
	};

	class ShaderStorageBuffer {
	public:
		virtual ~ShaderStorageBuffer() = default;
		virtual void Bind(uint32_t bindingPoint) const = 0;
		virtual void Unbind() const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void ClearSSBO(uint32_t size) = 0;
		virtual void* GetData() = 0;
		static ShaderStorageBuffer* Create(uint32_t size, const void* data = nullptr);
	};

	class DispatchIndirectBuffer {
	public:
		DispatchIndirectBuffer(uint32_t bufferId);
		~DispatchIndirectBuffer();
		void Bind() const;
		void Unbind() const;
	private:
		uint32_t m_BufferId;
	};
}
