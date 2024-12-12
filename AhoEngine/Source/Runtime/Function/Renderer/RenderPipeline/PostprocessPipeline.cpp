#include "Ahopch.h"
#include "PostprocessPipeline.h"
#include "Runtime/Resource/FileWatcher/FileWatcher.h"
#include "Runtime/Function/Renderer/Renderer.h"

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	void PostprocessPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_PostProcess;

		m_DrawSelectedPass = SetupDrawSelectedPass();
		m_DrawSelectedOutlinePass = SetupDrawSelectedOutlinePass();
		m_FXAAPass = SetupFXAAPass();

		m_DrawSelectedOutlinePass->RegisterTextureBuffer({ m_DrawSelectedPass->GetTextureBuffer(TexType::Result), TexType::Entity });
		m_FXAAPass->RegisterTextureBuffer({ m_DrawSelectedOutlinePass->GetTextureBuffer(TexType::Result), TexType::Result });

		RegisterRenderPass(m_DrawSelectedPass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_DrawSelectedOutlinePass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_FXAAPass.get(), RenderDataType::ScreenQuad);

		m_RenderResult = m_FXAAPass->GetTextureBuffer(TexType::Result);
	}

	void PostprocessPipeline::SetInput(Texture* tex) {
		m_Input = tex;
		m_DrawSelectedOutlinePass->RegisterTextureBuffer({ m_Input, TexType::Result });
	}

	std::unique_ptr<RenderPass> PostprocessPipeline::SetupDrawSelectedOutlinePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				shader->SetUint("u_SelectedEntityID", RendererGlobalState::g_SelectedEntityID);
				shader->SetBool("u_IsEntityIDValid", RendererGlobalState::g_IsEntityIDValid);

				for (const auto& data : renderData) {
					data->Bind(shader);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("Postprocessing");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto shaderPath = g_CurrentPath / "ShaderSrc" / "Postprocessing.glsl";
		auto shader = Shader::Create(shaderPath);

		pass->SetShader(shader);
		TexSpec texSpecColor;
		texSpecColor.debugName = "drawOutline";

		texSpecColor.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::PostProcessing);
		return pass;
	}

	std::unique_ptr<RenderPass> PostprocessPipeline::SetupDrawSelectedPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				if (RendererGlobalState::g_IsEntityIDValid && RendererGlobalState::g_SelectedData) {
					shader->Bind();
					renderTarget->EnableAttachments(0);
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					const auto& data = RendererGlobalState::g_SelectedData;
					shader->SetUint("u_EntityID", data->GetEntityID());
					data->Bind(shader);
					data->IsInstanced() ? RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()) : RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
					renderTarget->Unbind();
					shader->Unbind();
				}
			});

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("DrawSelectedPass");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pickingShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "DrawSelected.glsl");
		pass->SetShader(pickingShader);
		TexSpec spec;
		spec.debugName = "DrawSelectedSeperately";
		spec.internalFormat = TexInterFormat::UINT;
		spec.dataFormat = TexDataFormat::UINT;
		spec.dataType = TexDataType::UnsignedInt;
		spec.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { spec });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::DrawSelected);
		return pass;
	}

	std::unique_ptr<RenderPass> PostprocessPipeline::SetupFXAAPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				for (const auto& data : renderData) {
					data->Bind(shader);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("FXAAPass");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "FXAA.glsl");
		pass->SetShader(shader);

		TexSpec texSpecColor; texSpecColor.type = TexType::Result;
		texSpecColor.debugName = "FXAA";

		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::FXAA);
		return pass;
	}

}
