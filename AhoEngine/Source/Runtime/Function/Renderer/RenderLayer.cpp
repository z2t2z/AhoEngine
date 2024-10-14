#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include <imgui.h>

namespace Aho {
	RenderLayer::RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void RenderLayer::OnAttach() {
		AHO_CORE_INFO("Renderer on attach");
		SetupForwardRenderPipeline();
	}

	void RenderLayer::OnDetach() {

	}

	void RenderLayer::OnUpdate(float deltaTime) {
		m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->BindSceneDataUBO(m_UBO);
		m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(1)->BindSceneDataUBO(m_UBO);
		//m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(2)->BindSceneDataUBO(m_UBO);
		m_Renderer->Render();
	}

	void RenderLayer::OnImGuiRender() {
		/* In game UI logic, HUD or something */
	}

	void RenderLayer::OnEvent(Event& e) {
		/* Parameters on changed event */
		if (e.GetEventType() == EventType::UploadRenderData) {
			e.SetHandled();
			auto ee = (UploadRenderDataEvent*)&(e);
			AHO_CORE_WARN("Recieving a UploadRenderDataEvent!");
			auto renderData = ee->GetRawData();
			for (auto pipeLine : *m_Renderer) {
				pipeLine->AddRenderData(renderData);
			}
		}
	}


	void RenderLayer::SetupForwardRenderPipeline() {
		RenderPipelineDefault* pipeline = new RenderPipelineDefault();
		pipeline->AddRenderPass(SetupMainPass());
		auto depthPass = SetupDepthPass();
		auto depthMapFBO = depthPass->GetRenderTarget();
		pipeline->AddRenderPass(depthPass);
		auto debugPass = SetupDebugPass(depthMapFBO);
		pipeline->AddRenderPass(debugPass);
		m_Renderer->SetCurrentRenderPipeline(pipeline);
	}

	RenderPass* RenderLayer::SetupDebugPass(const std::shared_ptr<Framebuffer>& fbo) {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		RenderPassDefault* debugPass = new RenderPassDefault();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& lastFBO) {
			AHO_CORE_ASSERT(lastFBO, "Framebuffer is not set correctly in debug pass");
			uint32_t texOffset = 0u;
			lastFBO->GetDepthTexture()->Bind(texOffset);
			shader->SetInt("u_DepthMap", texOffset++);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		debugPass->SetRenderCommand(cmdBuffer);
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		const auto debugShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMapDebug.glsl");
		debugPass->SetShader(debugShader);
		debugPass->SetRenderPassType(RenderPassType::Debug);
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecColor.internalFormat = FBInterFormat::RGBA8;
		texSpecColor.dataFormat = FBDataFormat::RGBA;
		texSpecColor.dataType = FBDataType::UnsignedByte;
		texSpecColor.target = FBTarget::Texture2D;
		texSpecColor.wrapModeS = FBWrapMode::Clamp;
		texSpecColor.wrapModeT = FBWrapMode::Clamp;
		texSpecColor.filterModeMin = FBFilterMode::Nearest;
		texSpecColor.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1280u, 720u, { texSpecColor, texSpecDepth });
		auto FBO = Framebuffer::Create(fbSpec);
		debugPass->SetRenderTarget(FBO);
		return debugPass;
	}
	
	RenderPass* RenderLayer::SetupMainPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& lastFBO) {
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "Shader.glsl").string();
		auto shader = Shader::Create(FileName);
		RenderPassDefault* renderPass = new RenderPassDefault();
		if (shader->IsCompiled()) {
			renderPass->SetShader(std::move(shader));
		}
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecColor.internalFormat = FBInterFormat::RGBA8;
		texSpecColor.dataFormat = FBDataFormat::RGBA;
		texSpecColor.dataType = FBDataType::UnsignedByte;
		texSpecColor.target = FBTarget::Texture2D;
		texSpecColor.wrapModeS = FBWrapMode::Clamp;
		texSpecColor.wrapModeT = FBWrapMode::Clamp;
		texSpecColor.filterModeMin = FBFilterMode::Nearest;
		texSpecColor.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1280u, 720u, { texSpecColor, texSpecColor, texSpecDepth });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupDepthPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Framebuffer>& lastFBO) {
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		RenderPassDefault* depthRenderPass = new RenderPassDefault();
		depthRenderPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto depthShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMap.glsl");
		depthRenderPass->SetShader(depthShader);
		FBTextureSpecification texSpecDepth;
		texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecDepth.internalFormat = FBInterFormat::Depth24;	// ......
		texSpecDepth.dataType = FBDataType::Float;
		texSpecDepth.wrapModeS = FBWrapMode::Clamp;
		texSpecDepth.wrapModeT = FBWrapMode::Clamp;
		texSpecDepth.filterModeMin = FBFilterMode::Nearest;
		texSpecDepth.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1024u, 1024u, { texSpecDepth });  // Hardcode fow now
		auto depthFBO = Framebuffer::Create(fbSpec);
		int id = depthFBO->GetDepthTexture()->GetTextureID();
		AHO_CORE_TRACE("depthFBO texture ID {}", id);
		depthRenderPass->SetRenderTarget(depthFBO);
		return depthRenderPass;
	}
}