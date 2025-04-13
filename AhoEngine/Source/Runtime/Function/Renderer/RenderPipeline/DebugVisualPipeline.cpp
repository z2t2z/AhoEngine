#include "Ahopch.h"
#include "DebugVisualPipeline.h"

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	DebugVisualPipeline::DebugVisualPipeline() {
		Initialize();
	}
	
	void DebugVisualPipeline::Initialize() {
		m_LightVisualPass = SetupLightVisualPass();
		RegisterRenderPass(m_LightVisualPass.get(), RenderDataType::UnitCircle);
		m_RenderResult = m_LightVisualPass->GetTextureBuffer(TexType::Result);
	}

	//void DebugVisualPipeline::SetRenderTarget(RenderPassType type, const std::shared_ptr<Framebuffer>& fbo) {
	//	auto pass = GetRenderPass(type);
	//	pass->SetRenderTarget(fbo);
	//	m_RenderResult = m_LightVisualPass->GetTextureBuffer(TexType::Result);
	//}
	
	std::unique_ptr<RenderPass> DebugVisualPipeline::SetupLightVisualPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				//RenderCommand::Clear(ClearFlags::Color_Buffer);

				static glm::vec3 lightPos(0.0);

				for (const auto& data : renderData) {
					data->Bind(shader);
					for (int i = 0; i < 3; i++) {
						shader->SetInt("u_RotateIndex", i);
						RenderCommand::DrawLine(data->GetVAO());
					}
					data->Unbind();
				}

				RenderCommand::CheckError();
				renderTarget->Unbind();
				shader->Unbind();
			});
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("LightVisual");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "DebugVisual.glsl");
		AHO_CORE_ASSERT(shader->IsCompiled());

		pass->SetShader(shader);

		TexSpec texSpecColor; texSpecColor.type = TexType::Result;
		texSpecColor.debugName = "LightVisual";

		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::DebugVisual);
		return pass;
	}
}