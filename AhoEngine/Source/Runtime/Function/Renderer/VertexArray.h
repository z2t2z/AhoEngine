#pragma once

#include "Runtime/Resource/ResourceType/ResourceType.h"
#include <vector>
#include <memory>
#include "Buffer.h"

namespace Aho {
	struct Mesh;

	class VertexArray {
	public:
		virtual ~VertexArray() {}
		virtual void Init(const Mesh& mesh) = 0;
		virtual void Init(const std::shared_ptr<MeshInfo>& meshInfo) = 0;
		virtual void Init(const std::shared_ptr<SkeletalMeshInfo>& meshInfo) = 0;
		virtual void Init(const std::shared_ptr<LineInfo>& lineInfo) = 0;
		virtual void SetInstancedTransform(const std::vector<glm::mat4>& transform, bool dynamicDraw) = 0;
		virtual void UpdateInstancedTransform(const std::vector<glm::mat4>& transform) = 0;
		virtual uint32_t GetInstanceAmount() const = 0;
		virtual void SetInstancedAmount(uint32_t amount) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
		//virtual std::vector<std::shared_ptr<VertexBuffer>> GetVertexBuffer() = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

		static VertexArray* Create(bool dynamicDraw = false);
	};
}
