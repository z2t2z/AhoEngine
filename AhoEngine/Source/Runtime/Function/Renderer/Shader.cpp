#include "Ahopch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Resource/FileWatcher/FileWatcher.h"

namespace Aho {

	/* ==================================== class : Shader ===================================== */
	std::shared_ptr<Shader> Shader::Create(const std::filesystem::path& filepath) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    
				AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				auto shader = std::make_shared<OpenGLShader>(filepath.string());
				auto& watcher = FileWatcher::getInstance();
				watcher.AddFileToWatch(filepath.string(), shader);
				return shader;
		}

		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::shared_ptr<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:    AHO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		AHO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	/* 
		==================================== class : ShaderLibrary, no use for now =====================================
	*/
	void ShaderLibrary::Add(const std::string& name, const std::shared_ptr<Shader>& shader) {
		AHO_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const std::shared_ptr<Shader>& shader) {
		auto& name = shader->GetName();
		Add(name, shader);
	}

	std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& filepath) {
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath) {
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name) {
		AHO_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const {
		return m_Shaders.contains(name);
	}

}