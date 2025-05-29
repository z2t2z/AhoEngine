#include "Ahopch.h"
#include "RenderPassBase.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
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
		auto assetManger		= g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs				= g_RuntimeGlobalCtx.m_EntityManager;
		auto resourceManager	= g_RuntimeGlobalCtx.m_Resourcemanager;
		auto shaderAsset		= assetManger->_LoadAsset<ShaderAsset>(ecs, ShaderOptions(config.shaderPath));
		auto shaderResource		= resourceManager->LoadShaderResource(shaderAsset, ShaderFeature::NONE);
		m_Attachments			= config.textureAttachments;
		m_Shader				= shaderResource.get();
		m_Execute				= std::move(config.func);
		m_Name					= config.passName;

		//AHO_CORE_ASSERT(!config.textureAttachments.empty(), "No texture attachments provided.");
		if (!config.textureAttachments.empty()) {
			int width = config.textureAttachments[0]->GetWidth();
			int height = config.textureAttachments[0]->GetHeight();
			m_Framebuffer = std::make_unique<OpenGLFramebuffer>(config.textureAttachments, width, height);
		}
		else {
			AHO_CORE_WARN("No texture attachments provided.", m_Name);
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

	bool RenderPassBase::Resize(uint32_t width, uint32_t height) const {
		return m_Framebuffer->_Resize(width, height);
	}

	void RenderPassBase::BindRegisteredTextureBuffers(uint32_t& slot) const {
		for (const auto& [texture, name] : m_TextureBuffers) {
			RenderCommand::BindTextureUnit(slot, texture->GetTextureID());
			m_Shader->SetInt(name, slot++);
		}
	}
}
