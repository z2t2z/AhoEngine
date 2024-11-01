#include "Ahopch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"

namespace Aho {
	std::shared_ptr<Texture2D> Texture2D::Create(const TexSpec& specification) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLTexture2D>(specification);
		}
		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path, bool filpOnLoad, bool grayScale) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLTexture2D>(path, filpOnLoad, grayScale);
		}
		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::shared_ptr<Texture2D> Texture2D::Create() {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLTexture2D>();
		}
		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}