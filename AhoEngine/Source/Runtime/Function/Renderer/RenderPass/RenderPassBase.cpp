#include "Ahopch.h"
#include "RenderPassBase.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"


namespace Aho {
	uint32_t RenderPassBase::s_DummyVAO = 0;

	RenderPassBase::~RenderPassBase() {
		if (s_DummyVAO) {
			glDeleteVertexArrays(1, &s_DummyVAO);
		}
	}

	void RenderPassBase::Setup(RenderPassConfig& config) {
		auto assetManger = g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs		 = g_RuntimeGlobalCtx.m_EntityManager;
		auto shaderAsset = assetManger->_LoadAsset<ShaderAsset>(ecs, ShaderOptions(config.shaderPath));
		m_Shader		 = shaderAsset->GetShader().get();
		m_Execute		 = std::move(config.func);
		m_Name			 = config.passName;
		if (config.textureAttachments.empty()) {
			m_Framebuffer = std::make_unique<OpenGLFramebuffer>(config.textureAttachments);
		}
		else {
			int width = config.textureAttachments[0]->GetWidth();
			int height = config.textureAttachments[0]->GetHeight();
			m_Framebuffer = std::make_unique<OpenGLFramebuffer>(config.textureAttachments, width, height);
		}

		if (s_DummyVAO == 0) {
			glGenVertexArrays(1, &s_DummyVAO);
		}
	}

	void RenderPassBase::Execute() {
		RenderCommand::PushDebugGroup(m_Name);
		m_Execute(*this);
		RenderCommand::PopDebugGroup();
	}

	void RenderPassBase::BindRegisteredTextureBuffers(uint32_t& slot) {
		for (const auto& [textureID, name] : m_TextureBuffers) {
			RenderCommand::BindTextureUnit(slot, textureID);
			m_Shader->SetInt(name, slot++);
		}
	}
}
