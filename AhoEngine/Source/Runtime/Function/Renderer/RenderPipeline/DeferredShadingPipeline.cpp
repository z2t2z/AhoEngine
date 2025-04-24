#include "Ahopch.h"
#include "DeferredShadingPipeline.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Renderer/Renderer.h"

namespace Aho {

	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	void DeferredShadingPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_DeferredShading;

		m_ShadowMapPass = SetupShadowMapPass();
		m_SSAOPass		= SetupSSAOPass();
		m_GBufferPass	= SetupGBufferPass();
		m_BlurRPass		= SetupBlurRPass();
		m_ShadingPass	= SetupShadingPass();
	
		m_SSAOPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Position), TexType::Position });
		m_SSAOPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Normal), TexType::Normal });
		m_SSAOPass->RegisterTextureBuffer({ Utils::CreateNoiseTexture(32), TexType::Noise });

		m_BlurRPass->RegisterTextureBuffer({ m_SSAOPass->GetTextureBuffer(TexType::AO), TexType::Result });

		m_ShadingPass->RegisterTextureBuffer({ m_ShadowMapPass->GetTextureBuffer(TexType::Depth), TexType::LightDepth });
		m_ShadingPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Depth), TexType::Depth });
		m_ShadingPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Position), TexType::Position });
		m_ShadingPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Normal), TexType::Normal });
		m_ShadingPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::Albedo), TexType::Albedo });
		m_ShadingPass->RegisterTextureBuffer({ m_GBufferPass->GetTextureBuffer(TexType::PBR), TexType::PBR });  // PBR param, metalic and roughness in rg channels respectively
		m_ShadingPass->RegisterTextureBuffer({ m_BlurRPass->GetTextureBuffer(TexType::Result), TexType::AO });

		RegisterRenderPass(m_ShadowMapPass.get(), RenderDataType::SceneData);
		RegisterRenderPass(m_GBufferPass.get(), RenderDataType::SceneData);
		RegisterRenderPass(m_SSAOPass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_BlurRPass.get(), RenderDataType::ScreenQuad);
		RegisterRenderPass(m_ShadingPass.get(), RenderDataType::ScreenQuad);

		m_RenderResult = m_ShadingPass->GetTextureBuffer(TexType::Result);
	}

	void DeferredShadingPipeline::SetEnvLightState(bool state) {
		if (state && m_EnvLightState) {
			return;
		}
		if (state && !m_EnvLightState) {
			m_EnvLightState = true;
			auto shader = m_ShadingPass->GetShader();
			shader->Bind();
			shader->SetBool("u_SampleEnvLight", true);
			return;
		}
		if (!state && !m_EnvLightState) {
			return;
		}
		m_EnvLightState = state;
		auto shader = m_ShadingPass->GetShader();
		shader->Bind();
		shader->SetBool("u_SampleEnvLight", false);
	}

	std::unique_ptr<RenderPass> DeferredShadingPipeline::SetupShadowMapPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Depth_Buffer);
				for (const auto& data : renderData) {
					if (data->IsDebug() || !data->ShouldBeRendered()) {
						continue;
					}
					data->Bind(shader);
					if (data->IsInstanced()) { // TODO: move to debug
						shader->SetBool("u_IsInstanced", true);
						RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount());
						shader->SetBool("u_IsInstanced", false);
					}
					else {
						RenderCommand::DrawIndexed(data->GetVAO());
					}
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("ShadowMapPass");
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto depthShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "ShadowMap.glsl");
		pass->SetShader(depthShader);
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth32; depth.dataFormat = TexDataFormat::DepthComponent;
		depth.type = TexType::Depth;
		FBSpec fbSpec(4096, 4096, { depth });  // Hardcode fow now
		auto FBO = Framebuffer::Create(fbSpec);
		FBO->SetShouldResizeWithViewport(false);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::Depth);
		return pass;
	}

	std::unique_ptr<RenderPass> DeferredShadingPipeline::SetupGBufferPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->Bind();
				RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer | ClearFlags::Stencil_Buffer);

				for (const auto& data : renderData) {
					if (!data->ShouldBeRendered()) {
						continue;
					}
					
					if (data->GetWriteStencil()) {
						glEnable(GL_STENCIL_TEST);
						glStencilFunc(GL_ALWAYS, 1, 0xFF); 
						glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
						glStencilMask(0xFF);
					}

					data->Bind(shader);

					shader->SetUint("u_EntityID", data->GetEntityID());
					if (RendererGlobalState::g_SelectedEntityID == data->GetEntityID()) {
						RendererGlobalState::g_SelectedData = data;
					}

					if (data->IsInstanced()) {
						shader->SetBool("u_IsInstanced", true);
						RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount());
						shader->SetBool("u_IsInstanced", false);
					}
					else {
						RenderCommand::DrawIndexed(data->GetVAO());
					}
					data->Unbind();

					if (data->GetWriteStencil()) {
						glStencilMask(0xFF);
						glDepthMask(GL_TRUE);
						glDisable(GL_STENCIL_TEST);
					}
				}

				renderTarget->Unbind();
				shader->Unbind();
			});

		auto shader = Shader::Create((g_CurrentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("GbufferPass");
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24Stencil8; depth.dataType = TexDataType::UnsignedInt248;
		depth.dataFormat = TexDataFormat::DepthStencil; depth.type = TexType::Depth; depth.debugName = "depth";

		TexSpec positionAttachment;
		positionAttachment.debugName = "position";
		positionAttachment.internalFormat = TexInterFormat::RGBA16F;
		positionAttachment.dataFormat = TexDataFormat::RGBA;
		positionAttachment.dataType = TexDataType::Float;
		positionAttachment.filterModeMin = TexFilterMode::Nearest;
		positionAttachment.filterModeMag = TexFilterMode::Nearest;
		positionAttachment.type = TexType::Position;

		TexSpec normalAttachment = positionAttachment;
		normalAttachment.debugName = "normal";
		normalAttachment.wrapModeS = TexWrapMode::None;
		normalAttachment.type = TexType::Normal;

		TexSpec albedoAttachment;
		albedoAttachment.debugName = "albedo";
		albedoAttachment.type = TexType::Albedo;

		TexSpec entityAttachment;
		entityAttachment.debugName = "entity";
		entityAttachment.internalFormat = TexInterFormat::UINT;
		entityAttachment.dataFormat = TexDataFormat::UINT;
		entityAttachment.dataType = TexDataType::UnsignedInt;
		entityAttachment.type = TexType::Entity;

		TexSpec pbrAttachment = positionAttachment;
		pbrAttachment.debugName = "pbr";
		pbrAttachment.internalFormat = TexInterFormat::RGBA16F; pbrAttachment.dataFormat = TexDataFormat::RGBA; // Don't use rbga8 cause it will be normalized!
		pbrAttachment.dataType = TexDataType::Float;
		pbrAttachment.type = TexType::PBR;

		FBSpec fbSpec(1280u, 720u, { positionAttachment, normalAttachment, albedoAttachment, pbrAttachment, entityAttachment, depth });

		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::SSAOGeo);
		return pass;
	}

	std::unique_ptr<RenderPass> DeferredShadingPipeline::SetupShadingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[this](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				shader->SetVec3("u_SunDir", m_SunDir);

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				for (const auto& data : renderData) {
					//data-> TODO: no needs to bind material uniforms
					data->Bind(shader, texOffset);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}

				renderTarget->Unbind();
				shader->Unbind();
			});

		std::string FileName = (g_CurrentPath / "ShaderSrc" / "pbrShader.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("ShadingPass");
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
		spec.debugName = "shadingResult";
		spec.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { spec });
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::Shading);
		return pass;
	}

	std::unique_ptr<RenderPass> DeferredShadingPipeline::SetupSSAOPass() {
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
					data->Bind(shader, texOffset);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		auto shader = Shader::Create((g_CurrentPath / "ShaderSrc" / "SSAO.glsl").string());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("SSAOPass");
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
		spec.debugName = "ssaoResult";
		spec.internalFormat = TexInterFormat::RED;
		spec.dataFormat = TexDataFormat::RED;
		spec.dataType = TexDataType::Float;
		spec.filterModeMin = TexFilterMode::Nearest;
		spec.filterModeMag = TexFilterMode::Nearest;
		spec.type = TexType::AO;

		FBSpec fbSpec(1280u, 720u, { spec });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::SSAO);
		return pass;
	}

	std::unique_ptr<RenderPass> DeferredShadingPipeline::SetupBlurRPass() {
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
					data->Bind(shader, texOffset);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		std::string FileName = (g_CurrentPath / "ShaderSrc" / "BlurR.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("BlurRPass");
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
		spec.debugName = "blurR";
		spec.internalFormat = TexInterFormat::RED;
		spec.dataFormat = TexDataFormat::RED;
		spec.dataType = TexDataType::Float;
		spec.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { spec });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::BlurR);
		return pass;
	}
}