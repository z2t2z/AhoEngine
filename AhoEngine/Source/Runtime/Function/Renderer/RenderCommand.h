#pragma once

#include "RendererAPI.h"
#include "VertexArrayr.h"
#include "RenderData.h"
#include "Framebuffer.h"
#include <memory>
#include <glad/glad.h>

namespace Aho {
	// OpenGL Dedicated
	class RenderCommand {
	public:
		inline static void SetClearColor(const glm::vec4& color) {
			glClearColor(color.r, color.g, color.b, color.a);
		}
		inline static void Clear(ClearFlags flags) {
			glClear((uint32_t)flags);
		}
		inline static void SetLineWidth(float width = 2.5f) {
			glLineWidth(2.5f);
		}
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, bool DrawLine = false) {
			glDrawElements(DrawLine ? GL_LINES : GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		inline static void DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t amount, bool DrawLine = false) {
			glDrawElementsInstanced(DrawLine ? GL_LINES : GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, amount);
		}
		inline static void SetDepthTest(bool state) {
			//s_RendererAPI->SetDepthTest(state);
			if (state) {
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}
		}
		inline static void SetPolygonModeLine() {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		inline static void SetPolygonModeFill() {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		inline static void SetViewport(uint32_t width, uint32_t height) {
			glViewport(0, 0, width, height);
		}
		inline static void BindRenderTarget(uint32_t attachmentOffset, uint32_t textureID, int mipmapLevel) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentOffset, GL_TEXTURE_2D, textureID, mipmapLevel);
		}
		inline static void DrawBuffers(int siz, uint32_t* data) {
			glDrawBuffers(siz, data);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

	class RenderCommandBuffer {
	public:
		RenderCommandBuffer() = default;
		void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData,	// Meshes to render
					 const std::shared_ptr<Shader>& shader, 						// Shader to use
					 const std::shared_ptr<Framebuffer>& renderTarget, 				// RenderTarget 
					 const std::vector<Texture*>& textureBuffers) const {			// G-Buffer textures
			for (const auto& command : m_Commands) {
				command(renderData, shader, textureBuffers, renderTarget);			// TODO: actually only one command is needed for now
			}
		}
		void AddCommand(const std::function<void(const std::vector<std::shared_ptr<RenderData>>&, 
						const std::shared_ptr<Shader>&, 
						const std::vector<Texture*>&, 
						const std::shared_ptr<Framebuffer>&)>& func) { m_Commands.push_back(func); }
	private:
		std::vector<std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData,	
									   const std::shared_ptr<Shader>& shader,						
									   const std::vector<Texture*>& textureBuffers,					
									   const std::shared_ptr<Framebuffer>& renderTarget)>> m_Commands;
	};
}