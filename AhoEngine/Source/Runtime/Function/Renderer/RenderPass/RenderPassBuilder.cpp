#include "Ahopch.h"
#include "RenderPassBuilder.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Renderer.h"


namespace Aho {
	RenderPassBuilder& RenderPassBuilder::Name(const std::string& name) {
		m_Cfg.passName = name;
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::Shader(const std::string& shaderPath) {
		m_Cfg.shaderPath = shaderPath;
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::Usage(ShaderUsage usage) {
		m_Cfg.usage = usage;
		return *this;
	}
	// Render Target Texture
	RenderPassBuilder& RenderPassBuilder::AttachTarget(const std::shared_ptr<_Texture>& tex) {
		m_Cfg.attachmentTargets.push_back(tex.get());
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::Input(const std::string& name, const std::shared_ptr<_Texture>& tex) {
		m_Cfg.inputTextures.emplace_back(name, tex.get());
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::Func(std::function<void(RenderPassBase&)> func) {
		m_Cfg.func = std::move(func);
		return *this;
	}

	std::unique_ptr<RenderPassBase> RenderPassBuilder::Build() {
		std::unique_ptr<RenderPassBase> renderPass = std::make_unique<RenderPassBase>(m_Cfg);
		g_RuntimeGlobalCtx.m_Renderer->RegisterRenderPassBase(renderPass.get());
		return std::move(renderPass);
	}

}