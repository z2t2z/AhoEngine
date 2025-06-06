#pragma once

#include "RendererAPI.h"
#include "VertexArray.h"
#include "RenderData.h"
#include "Framebuffer.h"
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Aho {
	// OpenGL Dedicated
	class RenderCommand {
	public:
		inline static void BindTextureUnit(uint32_t slot, uint32_t textureID) {
			glBindTextureUnit(slot, textureID);
		}
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
		inline static void DrawIndexed(VertexArray* vertexArray, bool DrawLine = false) {
			glDrawElements(DrawLine ? GL_LINES : GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		inline static void DrawLine(const std::shared_ptr<VertexArray>& vertexArray) {
			glDrawElements(GL_LINES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		inline static void DrawIndexedInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t amount, bool DrawLine = false) {
			glDrawElementsInstanced(DrawLine ? GL_LINES : GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, amount);
		}
		inline static void DrawArray() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		inline static void DataMemoryBarrier(GLint flags) {
			glMemoryBarrier(flags);
		}
		inline static void CullFace() {
			//glEnable(GL_CULL_FACE);    
			//glCullFace(GL_BACK);  
		}
		inline static void CheckError() {
			GLenum error = glGetError();
			if (error != GL_NO_ERROR) {
				//std::cout << "OpenGL Error: " << error << std::endl;
				AHO_CORE_ERROR("OPENGL ERROR:{}", error);
			}
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
		inline static void PushDebugGroup(const std::string& name) {
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, name.c_str());
		}
		inline static void PopDebugGroup() {
			glPopDebugGroup();
		}
		static glm::vec4 s_DefaultClearColor;
	private:
		static RendererAPI* s_RendererAPI;
	};

	class TextureBuffer;

	class RenderCommandBuffer {
	public:
		RenderCommandBuffer() = default;
		~RenderCommandBuffer() = default;

		RenderCommandBuffer(const std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData,
														const std::shared_ptr<Shader>& shader,
														const std::vector<TextureBuffer>& textureBuffers,
														const std::shared_ptr<Framebuffer>& renderTarget)>& cmd) {
			//m_Command = cmd;
			AddCommand(cmd);
		}
		void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData,	// Meshes to render
					 const std::shared_ptr<Shader>& shader, 						// Shader to use
					 const std::shared_ptr<Framebuffer>& renderTarget, 				// RenderTarget 
					 const std::vector<TextureBuffer>& textureBuffers) const {			// G-Buffer textures
			m_Command(renderData, shader, textureBuffers, renderTarget);			// TODO: actually only one command is needed for now
		}
		void AddCommand(const std::function<void(const std::vector<std::shared_ptr<RenderData>>&,
												const std::shared_ptr<Shader>&,
												const std::vector<TextureBuffer>&,
												const std::shared_ptr<Framebuffer>&)>& func) {
			m_Command = func;
		}
	private:
		std::function<void(const std::vector<std::shared_ptr<RenderData>>& renderData,
									   const std::shared_ptr<Shader>& shader,
									   const std::vector<TextureBuffer>& textureBuffers,
									   const std::shared_ptr<Framebuffer>& renderTarget)> m_Command;
	};
}