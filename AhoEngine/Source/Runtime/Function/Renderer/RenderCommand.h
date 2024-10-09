#pragma once

#include "RendererAPI.h"
#include "VertexArrayr.h"
#include "RenderData.h"
#include <memory>

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
		inline static void DrawBuffer(const uint32_t* bufferID) {
			s_RendererAPI->DrawBuffer(bufferID);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

	class RenderCommandBuffer {
	public:
		void Execute(const std::shared_ptr<RenderData>& data) const {
			for (const auto& command : m_Commands) {
				command(data);
			}
		}
		void AddCommand(const std::function<void(const std::shared_ptr<RenderData>)>& func) {
			m_Commands.push_back(func);
		}
	private:
		std::vector<std::function<void(const std::shared_ptr<RenderData>& data)>> m_Commands;

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
	};

}