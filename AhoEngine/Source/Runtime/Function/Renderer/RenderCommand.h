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
			//s_RendererAPI->SetClearColor(color);
			glClearColor(color.r, color.g, color.b, color.a);
		}
		inline static void Clear(ClearFlags flags) {
			//s_RendererAPI->Clear(flags);
			glClear((uint32_t)flags);
		}
		inline static void DrawLine(const std::shared_ptr<VertexArray>& vertexArray) {
			glLineWidth(2.5f);
			glDrawElements(GL_LINES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
			//s_RendererAPI->DrawIndexed(vertexArray);
			glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		inline static void DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t amount) {
			glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, amount);
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
		RenderCommandBuffer() { RenderCommand::SetClearColor(m_ClearColor); }
		void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData,	// Meshes to render
					 const std::shared_ptr<Shader>& shader, 						// Shader to use
					 const std::shared_ptr<Framebuffer>& renderTarget, 				// G-Buffer textures
					 const std::vector<Texture*>& textureBuffers,					// Render target
				     const void* ubo) const {										// Uniform block object
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::SetDepthTest(m_Depthtest);
			if (m_ClearFlags != ClearFlags::None) {
				RenderCommand::Clear(m_ClearFlags);
			}
			for (const auto& command : m_Commands) {
				command(renderData, shader, textureBuffers, renderTarget, ubo); // TODO: actually only one command is needed for now
			}
			renderTarget->Unbind();
			shader->Unbind();
		}
		void AddCommand(const std::function<void(const std::vector<std::shared_ptr<RenderData>>&, 
						const std::shared_ptr<Shader>&, 
						const std::vector<Texture*>&, 
						const std::shared_ptr<Framebuffer>&,
						const void*)>& func) { m_Commands.push_back(func); }
		virtual void SetDepthTest(bool state) { m_Depthtest = state; }
		virtual void SetClearColor(glm::vec4 color) { m_ClearColor = color; }
		virtual void SetClearFlags(ClearFlags flags) { m_ClearFlags = flags; }
	private:
		bool m_Depthtest{ true };
		glm::vec4 m_ClearColor{ 132.0f / 255.0f, 181.0f / 255.0f, 245.0f / 255.0f, 1.0f };
		ClearFlags m_ClearFlags{ ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer };
	private:
		std::vector<std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData,	
									   const std::shared_ptr<Shader>& shader,						
									   const std::vector<Texture*>& textureBuffers,					
									   const std::shared_ptr<Framebuffer>& renderTarget,			
									   const void* ubo)>> m_Commands;								
	};
}