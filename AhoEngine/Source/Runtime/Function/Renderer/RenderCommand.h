#pragma once

#include "RendererAPI.h"
#include "VertexArrayr.h"
#include "RenderData.h"
#include "Framebuffer.h"
#include <memory>
#include <glad/glad.h>

namespace Aho {
	class RenderCommand {
	public:
		inline static void SetClearColor(const glm::vec4& color) {
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear(ClearFlags flags) {
			s_RendererAPI->Clear(flags);
		}
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

	class RenderCommandBuffer {
	public:
		void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& renderTarget, const std::vector<Texture*>& textureBuffers) const {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::SetClearColor(m_ClearColor);
			RenderCommand::Clear(m_ClearFlags);
			for (const auto& command : m_Commands) {
				command(renderData, shader, textureBuffers);
			}
			renderTarget->Unbind();
			shader->Unbind();
		}
		void AddCommand(const std::function<void(const std::vector<std::shared_ptr<RenderData>>&, const std::shared_ptr<Shader>&, const std::vector<Texture*>&)>& func) { m_Commands.push_back(func); }
		virtual void SetClearColor(glm::vec4 color) { m_ClearColor = color; }
		virtual void SetClearFlags(ClearFlags flags) { m_ClearFlags = flags; }
	private:
		glm::vec4 m_ClearColor{ 132.0f / 255.0f, 181.0f / 255.0f, 245.0f / 255.0f, 1.0f };
		ClearFlags m_ClearFlags{ ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer };
	private:
		std::vector<std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers)>> m_Commands;
	};
}