#include "Ahopch.h"
#include "RenderCommand.h"

#include "Runtime/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Aho {
	// constexpr?
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
	glm::vec4 RenderCommand::s_DefaultClearColor = glm::vec4{ 132.0f / 255.0f, 181.0f / 255.0f, 245.0f / 255.0f, 1.0f };
}