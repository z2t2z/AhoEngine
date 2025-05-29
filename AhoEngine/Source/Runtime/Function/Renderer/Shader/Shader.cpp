#include "Ahopch.h"
#include "Shader.h"
#include "../Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"

namespace Aho {

	/* ==================================== class : Shader ===================================== */
	std::shared_ptr<Shader> Shader::Create(const std::filesystem::path& filepath) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    
				AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				auto shader = std::make_shared<OpenGLShader>(filepath.string());
				return shader;
		}

		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}