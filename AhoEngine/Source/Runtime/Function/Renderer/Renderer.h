#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "Runtime/Function/Camera/Camera.h"
#include "RenderPipeline.h"
#include <memory>
#include <typeindex>
#include <glm/glm.hpp>

namespace Aho {
	class Renderer {
	public:
		Renderer();
		~Renderer() = default;
		void Render() {
			m_CurrentPipeline->Execute();
		}
		void SetRenderPipeline(const std::shared_ptr<RenderPipeline> pl) { m_CurrentPipeline = pl; }
		std::shared_ptr<RenderPipeline> GetCurrentRenderPipeline() { return m_CurrentPipeline; }
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		std::shared_ptr<RenderPipeline> m_CurrentPipeline{ nullptr };
	};
}

