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
		void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& renderTarget, const std::shared_ptr<Framebuffer>& lastFBO = nullptr) const {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::SetClearColor(m_ClearColor);
			RenderCommand::Clear(m_ClearFlags);
			for (const auto& command : m_Commands) {
				command(renderData, shader, lastFBO);
			}
			renderTarget->Unbind();
			shader->Unbind();
		}
		void AddCommand(const std::function<void(const std::vector<std::shared_ptr<RenderData>>&, const std::shared_ptr<Shader>&, const std::shared_ptr<Framebuffer>&)>& func) { m_Commands.push_back(func); }
		virtual void SetClearColor(glm::vec4 color) { m_ClearColor = color; }
		virtual void SetClearFlags(ClearFlags flags) { m_ClearFlags = flags; }
	private:
		glm::vec4 m_ClearColor{ 132.0f / 255.0f, 181.0f / 255.0f, 245.0f / 255.0f, 1.0f };
		ClearFlags m_ClearFlags{ ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer };
	private:
		std::vector<std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& lastFBO)>> m_Commands;
	};

	//class RenderCommandBuffer {
	//public:
	//	void Execute(const std::shared_ptr<RenderData>& data, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& fbo = nullptr) const {
	//		for (const auto& command : m_Commands) {
	//			command(data, shader, fbo);
	//		}
	//	}
	//	void AddCommand(const std::function<void(const std::shared_ptr<RenderData>, const std::shared_ptr<Shader>, const std::shared_ptr<Framebuffer>)>& func) {
	//		m_Commands.push_back(func);
	//	}
	//private:
	//	std::vector<std::function<void(const std::shared_ptr<RenderData>& data, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& fbo)>> m_Commands;
	//};
	//TODO: think of doing things this way
	//	template <typename Func, typename... Args>
	//	void AddCommand(Func&& func, Args&&... args) {
	//		commands.emplace_back(std::forward<Func>(func), std::forward<Args>(args)...);
	//	}
	//	void Execute() const {
	//		for (const auto& command : m_Commands) {
	//			//command();
	//		}
	//	}
	//	template <typename Func, typename... Args>
	//	void StoreCommand(Func&& func, Args&&... args) {
	//		commands.emplace_back([this, func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() {
	//			func(std::get<Args>(args)...);
	//		});
	//	}
}