#pragma once

#include "Runtime/Resource/ResourceType/ResourceType.h"
#include <vector>
#include <memory>
#include "Buffer.h"

namespace Aho {
	class VertexArray {
	public:
		virtual ~VertexArray() {}
		virtual void Init(const std::shared_ptr<MeshInfo>& meshInfo) = 0;
		virtual void Init(const std::shared_ptr<SkeletalMeshInfo>& meshInfo) = 0;
		virtual void Init(const std::shared_ptr<LineInfo>& lineInfo) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, uint32_t& offset) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
		//virtual std::vector<std::shared_ptr<VertexBuffer>> GetVertexBuffer() = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

		static VertexArray* Create(bool dynamicDraw = false);
	};
}
