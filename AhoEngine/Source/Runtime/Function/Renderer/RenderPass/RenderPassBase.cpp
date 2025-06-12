#include "Ahopch.h"
#include "RenderPassBase.h"
#include "Runtime/Function/Renderer/GpuTimer.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/Asset.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Aho {
	uint32_t RenderPassBase::s_DummyVAO = 0;

	RenderPassBase::~RenderPassBase() {
		if (s_DummyVAO) {
			glDeleteVertexArrays(1, &s_DummyVAO);
			s_DummyVAO = 0;
		}
	}

	void RenderPassBase::Setup(RenderPassConfig config) {
		auto assetManger		= g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs				= g_RuntimeGlobalCtx.m_EntityManager;
		auto resourceManager	= g_RuntimeGlobalCtx.m_Resourcemanager;
		auto shaderAsset		= assetManger->_LoadAsset<ShaderAsset>(ecs, ShaderOptions(config.shaderPath, config.usage));
		auto shaderResource		= resourceManager->LoadShaderResource(shaderAsset, ShaderFeature::NONE);
		m_Attachments			= config.attachmentTargets;
		m_Shader				= shaderResource.get();
		m_Execute				= std::move(config.func);
		m_Name					= config.passName;
		m_InputTextures			= config.inputTextures;

		if (!config.attachmentTargets.empty()) {
			int width = config.attachmentTargets[0]->GetWidth();
			int height = config.attachmentTargets[0]->GetHeight();
			m_Framebuffer = std::make_unique<OpenGLFramebuffer>(config.attachmentTargets, width, height);
		}
		else {
			AHO_CORE_WARN("No texture attachments provided in pass: {}, won't create fb.", m_Name);
		}
		if (s_DummyVAO == 0) {
			glGenVertexArrays(1, &s_DummyVAO);
		}

		m_GpuTimer = std::make_unique<GpuTimer>();
	}

	void RenderPassBase::Execute() {
		RenderCommand::PushDebugGroup(m_Name);

		m_GpuTimer->Begin();
		m_Execute(*this);
		m_GpuTimer->End();

		RenderCommand::PopDebugGroup();
		m_GpuTimer->Update(); 
		m_FrameTime = m_GpuTimer->GetLatestTime(); // ms
	}

	bool RenderPassBase::Resize(uint32_t width, uint32_t height) const {
		if (m_Framebuffer) {
			return m_Framebuffer->Resize(width, height);
		}
		return false;
	}

	void RenderPassBase::BindRegisteredTextureBuffers(uint32_t& slot) const {
		for (const auto& [name, texture] : m_InputTextures) {
			RenderCommand::BindTextureUnit(slot, texture->GetTextureID());
			m_Shader->SetInt(name, slot++);
		}
	}
}
