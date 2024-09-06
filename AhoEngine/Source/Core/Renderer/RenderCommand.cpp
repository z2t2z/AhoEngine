#include "Ahopch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Aho {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}