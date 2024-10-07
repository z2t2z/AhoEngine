#pragma once

#include "RenderCommand.h"
#include "RendererAPI.h"

//#include "Renderer.h"

namespace Aho {
	/* 
		Rendering hardware interface 
		Referred to Piccolo
	*/

	class RHI {
	public:
		~RHI() = default;

		virtual bool CreatePipeline() = 0;
		virtual bool CreateFrameBuffer() = 0;
		virtual bool CreateTexture() = 0;
		virtual bool CreateVertexArray() = 0;
		virtual bool CreateIndexArray() = 0;

		virtual bool CreateShaderStorageBuffer() = 0;
		virtual bool CreateIndexBuffer() = 0;

		// RenderCommand
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Flush() = 0;
		virtual void Draw(const std::shared_ptr<VertexArray>& vertexArray) = 0;

		// Shader

	private:
		static RendererAPI* s_RendererAPI;
	};
} // namespace Aho 