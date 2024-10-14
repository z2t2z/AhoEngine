#include "Ahopch.h"
#include "FrameBuffer.h"

#include "Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Aho {
	std::shared_ptr<Framebuffer> Framebuffer::Create(const FBSpecification& spec) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLFramebuffer>(spec);
		}
		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}