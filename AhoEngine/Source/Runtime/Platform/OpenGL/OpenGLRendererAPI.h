#pragma once

#include "Runtime/Function/Renderer/RendererAPI.h"

namespace Aho {
	class OpenGLRendererAPI : public RendererAPI {
	public:
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear(ClearFlags flags) override;
		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
		virtual void SetDepthTest(bool state) override;
	};
}
