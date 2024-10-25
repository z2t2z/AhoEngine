#include "Ahopch.h"
#include "VertexArrayr.h"

#include "Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLVertexArray.h"

namespace Aho {

	VertexArray* VertexArray::Create(bool dynamicDraw) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLVertexArray(dynamicDraw);
		}

		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
