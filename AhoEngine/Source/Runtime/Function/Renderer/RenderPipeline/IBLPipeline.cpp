#include "Ahopch.h"
#include "IBLPipeline.h"

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();
	static std::filesystem::path g_ShaderPath = std::filesystem::current_path() / "ShaderSrc";

	static glm::mat4 g_Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	static glm::mat4 g_Views[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	void IBLPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_IBL;

		m_RP_GenCubemapFromSphericalMap = SetupGenCubemapFromSphericalMapPass();
		m_RP_PrecomputeIrradiance = SetupPrecomputeIrradiancePass();
		m_RP_Prefiltering = SetupPrefilteredPass();
		m_RP_GenLUT = SetupGenLUTPass();

		RegisterRenderPass(m_RP_GenCubemapFromSphericalMap.get(), RenderDataType::UnitCube);
		RegisterRenderPass(m_RP_PrecomputeIrradiance.get(), RenderDataType::UnitCube);
		RegisterRenderPass(m_RP_Prefiltering.get(), RenderDataType::UnitCube);
		RegisterRenderPass(m_RP_GenLUT.get(), RenderDataType::UnitCube);
	}

	std::unique_ptr<RenderPass> IBLPipeline::SetupEquirectToCubePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("EquirectToCubePass");
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
				shader->Bind();

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				shader->SetMat4("u_Projection", g_Projection);
				renderTarget->Bind();
				for (int i = 0; i < 6; i++) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer);
					shader->SetMat4("u_View", g_Views[i]);
					renderTarget->BindCubeMap(renderTarget->GetTextureAttachments()[0], i);  // Project the spherical map to our cubemap
					for (const auto& data : renderData) {
						data->Bind(shader);
						RenderCommand::DrawIndexed(data->GetVAO());
						data->Unbind();
					}
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto shader = Shader::Create(g_ShaderPath / "IBL" / "EquirectToCube.glsl");
		pass->SetShader(shader);
		TexSpec spec; spec.type = TexType::CubeMap;
		spec.target = TexTarget::TextureCubemap;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(512, 512, { spec });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::GenCubemap);
		return pass;
	}

	std::unique_ptr<RenderPass> IBLPipeline::SetupGenCubemapFromSphericalMapPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("GenCubeMapFromHDRPass");
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
				shader->Bind();

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				shader->SetMat4("u_Projection", g_Projection);
				renderTarget->Bind();
				for (int i = 0; i < 6; i++) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer);
					shader->SetMat4("u_View", g_Views[i]);
					renderTarget->BindCubeMap(renderTarget->GetTextureAttachments()[0], i);  // Project the spherical map to our cubemap
					for (const auto& data : renderData) {
						data->Bind(shader);
						RenderCommand::DrawIndexed(data->GetVAO());
						data->Unbind();
					}
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "GenCubeMapFromHDR.glsl");
		pass->SetShader(shader);
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;
		TexSpec spec; spec.type = TexType::CubeMap;
		spec.target = TexTarget::TextureCubemap;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(512, 512, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::GenCubemap);
		return pass;
	}
	
	std::unique_ptr<RenderPass> IBLPipeline::SetupPrecomputeIrradiancePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("PrecomputeIrradiancePass");
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
				shader->Bind();

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				shader->SetMat4("u_Projection", g_Projection);
				renderTarget->Bind();
				for (int i = 0; i < 6; i++) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer);
					shader->SetMat4("u_View", g_Views[i]);
					renderTarget->BindCubeMap(renderTarget->GetTextureAttachments()[0], i);  // This is the cubemap we are going to write the calculated irradiance
					for (const auto& data : renderData) {
						data->Bind(shader, texOffset);
						RenderCommand::DrawIndexed(data->GetVAO());
						data->Unbind();
					}
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "IrradianceConv.glsl");
		pass->SetShader(shader);

		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;
		TexSpec spec; spec.type = TexType::Irradiance;
		spec.target = TexTarget::TextureCubemap;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(64, 64, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::PrecomputeIrradiance);
		return pass;
	}
	
	std::unique_ptr<RenderPass> IBLPipeline::SetupPrefilteredPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("PrefilteredPass");
		static uint32_t prefilteredRes = 1024u;
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				RenderCommand::Clear(ClearFlags::Color_Buffer);
				shader->Bind();

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				shader->SetMat4("u_Projection", g_Projection);
				renderTarget->Bind();

				uint32_t maxMipLevel = 5;
				for (uint32_t i = 0; i < maxMipLevel; i++) {
					uint32_t width = static_cast<uint32_t>(prefilteredRes * std::pow(0.5, i));
					uint32_t height = static_cast<uint32_t>(prefilteredRes * std::pow(0.5, i));
					RenderCommand::SetViewport(width, height);

					// TODO: Could be wrong, did not reset depth component's size
					float roughness = (float)i / (float)(maxMipLevel - 1);
					shader->SetFloat("u_Roughness", roughness);

					for (int j = 0; j < 6; j++) {
						RenderCommand::Clear(ClearFlags::Depth_Buffer);
						shader->SetMat4("u_View", g_Views[j]);
						renderTarget->BindCubeMap(renderTarget->GetTextureAttachments()[0], j, 0, i);  // Project the spherical map to our cubemap
						for (const auto& data : renderData) {
							data->Bind(shader);
							RenderCommand::DrawIndexed(data->GetVAO());
							data->Unbind();
						}
					}
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "Prefilter.glsl");
		pass->SetShader(shader);
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;
		TexSpec spec;
		spec.target = TexTarget::TextureCubemap; spec.width = spec.height = prefilteredRes;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		spec.filterModeMin = TexFilterMode::LinearMipmapLinear;
		spec.mipLevels = 5;
		spec.type = TexType::Prefiltering;
		FBSpec fbSpec(prefilteredRes, prefilteredRes, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::Prefilter);
		return pass;
	}
	
	std::unique_ptr<RenderPass> IBLPipeline::SetupGenLUTPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("BRDFLut");
		static bool Generated = false;
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				if (Generated) {
					return;
				}
				Generated = true;

				RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
				shader->Bind();
				renderTarget->Bind();

				for (const auto& data : renderData) {
					data->Bind(shader);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}

				renderTarget->Unbind();
				shader->Unbind();
			});

		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "GenLUT.glsl");
		pass->SetShader(shader);
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;
		TexSpec spec;
		spec.width = spec.height = 512;
		spec.internalFormat = TexInterFormat::RG16F; spec.dataFormat = TexDataFormat::RG; spec.dataType = TexDataType::Float; spec.type = TexType::BRDFLUT;
		FBSpec fbSpec(512, 512, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::GenLUT);
		return pass;
	}

}
