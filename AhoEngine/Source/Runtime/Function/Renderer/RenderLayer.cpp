#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
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
		//const auto& mainShader = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetShader();
		//mainShader->BindUBO(m_UBO); // TODO: set in render pass
		m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->BindSceneDataUBO(m_UBO);
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
			//for (const auto& data : ee->GetRawData()) {
			//	auto shader = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetShader();
			//	if (data->GetMaterial()) {
			//		data->GetMaterial()->SetShader(shader);
			//	}
			//}
			for (auto pipeLine : *m_Renderer) {
				for (auto renderPass : *pipeLine) {
					if (renderPass->GetRenderPassType() != RenderPassType::Debug) { // Debug pass usually draws a screen quad
						renderPass->AddRenderData(renderData);
					}
				}
			}
		}
	}
	void RenderLayer::SetupForwardRenderPipeline() {
		RenderPipelineDefault* pipeline = new RenderPipelineDefault();
		m_Renderer->SetCurrentRenderPipeline(pipeline);
		m_Renderer->GetCurrentRenderPipeline()->AddRenderPass(SetupMainPass());
		//m_Renderer->GetCurrentRenderPipeline()->AddRenderPass(SetupDepthPass());
		//m_Renderer->GetCurrentRenderPipeline()->AddRenderPass(SetupDebugPass());
	}

	RenderPass* RenderLayer::SetupDebugPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([&](const std::shared_ptr<RenderData>& data, const std::shared_ptr<Shader>& shader) {
			data->ShouldBindMaterial(true);
			data->Bind(shader);
			RenderCommand::DrawIndexed(data->GetVAO());
			data->Unbind();
		});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		const auto debugShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMapDebug.glsl");
		RenderPassForward* debugPass = new RenderPassForward();
		debugPass->SetRenderCommand(cmdBuffer);
		debugPass->SetShader(debugShader);
		FBSpecification fbSpec;
		fbSpec.Width = 1280u;
		fbSpec.Height = 720u;
		auto FBO = Framebuffer::Create(fbSpec);
		FBTextureSpecification texSpec;
		texSpec.TextureFormat = FBTextureFormat::RGBA8;
		texSpec.internalFormat = FBInterFormat::RGBA8;
		texSpec.dataFormat = FBDataFormat::RGBA;
		texSpec.dataType = FBDataType::UnsignedByte;
		texSpec.target = FBTarget::Texture2D;
		texSpec.wrapModeS = FBWrapMode::Clamp;
		texSpec.wrapModeT = FBWrapMode::Clamp;
		texSpec.filterModeMin = FBFilterMode::Nearest;
		texSpec.filterModeMag = FBFilterMode::Nearest;
		FBO->Bind();
		FBO->AddColorAttachment(texSpec);
		FBO->Invalidate();
		FBO->Unbind();
		debugPass->SetRenderTarget(FBO);
		return debugPass;
	}
	
	RenderPass* RenderLayer::SetupMainPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([&](const std::shared_ptr<RenderData>& data, const std::shared_ptr<Shader>& shader) {
			data->ShouldBindMaterial(true);
			data->Bind(shader);
			RenderCommand::DrawIndexed(data->GetVAO());
			data->Unbind();
		});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		const auto shader = Shader::Create(currentPath / "ShaderSrc" / "Shader.glsl");
		RenderPassForward* renderPass = new RenderPassForward();
		renderPass->SetRenderCommand(cmdBuffer);
		renderPass->SetShader(shader);
		FBSpecification fbSpec;
		fbSpec.Width = 1280u;
		fbSpec.Height = 720u;
		auto FBO = Framebuffer::Create(fbSpec);
		FBTextureSpecification texSpec;
		texSpec.TextureFormat = FBTextureFormat::RGBA8;
		texSpec.internalFormat = FBInterFormat::RGBA8;
		texSpec.dataFormat = FBDataFormat::RGBA;
		texSpec.dataType = FBDataType::UnsignedByte;
		texSpec.target = FBTarget::Texture2D;
		texSpec.wrapModeS = FBWrapMode::Clamp;
		texSpec.wrapModeT = FBWrapMode::Clamp;
		texSpec.filterModeMin = FBFilterMode::Nearest;
		texSpec.filterModeMag = FBFilterMode::Nearest;
		FBO->Bind();
		FBO->AddColorAttachment(texSpec);
		FBO->AddColorAttachment(texSpec);
		FBO->Invalidate();
		FBO->Unbind();
		renderPass->SetRenderTarget(FBO);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupDepthPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([&](const std::shared_ptr<RenderData>& data, const std::shared_ptr<Shader>& shader) {
			data->ShouldBindMaterial(false);
			data->Bind(shader);
			RenderCommand::DrawIndexed(data->GetVAO());
			data->Unbind();
		});
		RenderPassForward* depthRenderPass = new RenderPassForward();
		depthRenderPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		const auto depthShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMap.glsl");
		FBTextureSpecification texSpec;
		texSpec.TextureFormat = FBTextureFormat::DEPTH24STENCIL8;
		texSpec.dataFormat = FBDataFormat::DepthComponent;
		texSpec.internalFormat = FBInterFormat::Depth24;	// ......
		texSpec.dataType = FBDataType::Float;
		texSpec.wrapModeS = FBWrapMode::Repeat;
		texSpec.wrapModeT = FBWrapMode::Repeat;
		FBSpecification fbSpec;
		fbSpec.Width = fbSpec.Height = 1024u;
		fbSpec.Attachments.Attachments = std::vector<FBTextureSpecification>(1, texSpec);
		auto depthFBO = Framebuffer::Create(fbSpec);
		depthRenderPass->SetClearFlags(ClearFlags::Depth_Buffer);
		depthRenderPass->SetRenderTarget(depthFBO);
		depthRenderPass->SetShader(depthShader);
		return depthRenderPass;
	}
}