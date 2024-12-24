#include "Ahopch.h"
#include "RenderSkyPipeline.h"


namespace Aho {
	std::pair<float, float> RenderSkyPipeline::m_SunYawPitch{ 0.0f, 0.45f };

	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	void RenderSkyPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_RenderSky;

		m_AtmosParams.TopRadius		= 6460.0f;
		m_AtmosParams.BottomRadius	= 6360.0f;

		m_TransmittanceLutPass		= SetupTransmittanceLUTPass();
		m_MutiScattLutPass			= SetupMutiScattLutPass();
		m_SkyViewLutPass			= SetupSkyViewLutPass();

		m_MutiScattLutPass->RegisterTextureBuffer({ m_TransmittanceLutPass->GetTextureBuffer(TexType::Result), TexType::TransmittanceLUT });
		m_SkyViewLutPass->RegisterTextureBuffer({ m_TransmittanceLutPass->GetTextureBuffer(TexType::Result), TexType::TransmittanceLUT });
		m_SkyViewLutPass->RegisterTextureBuffer({ m_MutiScattLutPass->GetTextureBuffer(TexType::Result), TexType::MultiScattLUT });

		RegisterRenderPass(m_TransmittanceLutPass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_MutiScattLutPass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_SkyViewLutPass.get(), RenderDataType::ScreenQuad);

		m_RenderResult = m_SkyViewLutPass->GetTextureBuffer(TexType::Result);
	}

	std::unique_ptr<RenderPass> RenderSkyPipeline::SetupTransmittanceLUTPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);
				for (const auto& data : renderData) {
					data->Bind(shader);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("TransmittanceLUT");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pp = Shader::Create(g_CurrentPath / "ShaderSrc" / "PrecomputeAtmospheric.glsl");
		pass->SetShader(pp);
		TexSpec texSpecColor;
		texSpecColor.debugName = "transmittanceLUT";
		texSpecColor.type = TexType::Result;
		texSpecColor.internalFormat = TexInterFormat::RGBA16F;
		FBSpec fbSpec(256u, 64u, { texSpecColor });
		auto fbo = Framebuffer::Create(fbSpec);
		fbo->SetShouldResizeWithViewport(false);	  // do not resize with viewport
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::TransmittanceLUT);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderSkyPipeline::SetupMutiScattLutPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				// Sampler uniforms
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

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("MutiScattLutPass");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pp = Shader::Create(g_CurrentPath / "ShaderSrc" / "AtmosphericScattering" / "MutiScattLUT.glsl");
		pass->SetShader(pp);
		TexSpec texSpecColor;
		texSpecColor.debugName = "MutiScattLUT";
		texSpecColor.type = TexType::Result;
		texSpecColor.internalFormat = TexInterFormat::RGBA16F;
		FBSpec fbSpec(32u, 32u, { texSpecColor });
		auto fbo = Framebuffer::Create(fbSpec);
		fbo->SetShouldResizeWithViewport(false);	  // do not resize with viewport
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::MutiScattLUT);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderSkyPipeline::SetupSkyViewLutPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[this](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				shader->SetVec3("u_SunDir", m_SunDir);
				// Sampler uniforms
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

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("SkyLUTPass");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pp = Shader::Create(g_CurrentPath / "ShaderSrc" / "AtmosphericScattering" / "SkyLUT.glsl");
		pass->SetShader(pp);
		TexSpec texSpecColor;
		texSpecColor.debugName = "SkyViewLUT";
		texSpecColor.type = TexType::Result;
		texSpecColor.internalFormat = TexInterFormat::RGBA16F;
		FBSpec fbSpec(192u, 108u, { texSpecColor });
		auto fbo = Framebuffer::Create(fbSpec);
		fbo->SetShouldResizeWithViewport(false);	  // do not resize with viewport
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::SkyViewLUT);
		return pass;
	}

}
