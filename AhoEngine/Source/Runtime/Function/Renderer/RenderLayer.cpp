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
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([&](const std::shared_ptr<RenderData>& data) {
			data->Bind();
			RenderCommand::DrawIndexed(data->GetVAO());
			data->Unbind();
		});
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto shader = Shader::Create(currentPath / "ShaderSrc" / "Shader.glsl");
		RenderPassForward* renderPass = new RenderPassForward();
		renderPass->SetRenderCommand(cmdBuffer);
		renderPass->SetShader(shader);

		FBSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		const auto& FBO = Framebuffer::Create(fbSpec);
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
		RenderPipelineDefault* pipeline = new RenderPipelineDefault();
		pipeline->AddRenderPass(renderPass);
		m_Renderer->SetRenderPipeline(pipeline);
	}

	void RenderLayer::OnDetach() {

	}

	void RenderLayer::OnUpdate(float deltaTime) {
		const auto& mainShader = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetShader();
		mainShader->BindUBO(m_UBO); // TODO: set in render pass
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
			for (const auto& data : ee->GetRawData()) {
				auto shader = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetShader();
				if (data->GetMaterial()) {
					data->GetMaterial()->SetShader(shader);
				}
			}
			m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->AddRenderData(renderData);
		}
	}
}