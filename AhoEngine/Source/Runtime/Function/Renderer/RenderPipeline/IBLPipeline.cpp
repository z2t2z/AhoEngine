#include "Ahopch.h"
#include "IBLPipeline.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureUsage.h"
#include "Runtime/Function/Renderer/Texture/TextureConfig.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

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



	void _IBLPipeline::Initialize() {
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "IBL";

		// --- Generate cubemap from equirectangular map --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "Generate Cubemap From Equirectangular Map Pass";
			cfg.shaderPath = (shaderPathRoot / "EquirectToCube.glsl").string();

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* cubeMap = nullptr;
					_Texture* equiRec = nullptr;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.EnvTextureSkyBox) {
								iblComp.EnvTextureSkyBox = std::make_unique<_Texture>(TextureConfig::GetCubeMapTextureConfig());
								cubeMap = iblComp.EnvTextureSkyBox.get();
								equiRec = iblComp.EnvTexture;
							}
						}
					);
					if (!equiRec) {
						return;
					}

					// Write to cube map
					cubeMap->BindTextureImage(0);

					// Read from equirectangular map
					RenderCommand::BindTextureUnit(0, equiRec->GetTextureID());
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_EquirectangularMap", 0);
					shader->SetVec2("u_EquirectangularMapSize", { equiRec->GetWidth(), equiRec->GetHeight() });
					shader->SetFloat("u_CubemapFaceSize", cubeMap->GetWidth());

					static int group = 16;
					uint32_t face = cubeMap->GetWidth();
					uint32_t numGroups = (face + group - 1) / group;
					shader->DispatchCompute(numGroups, numGroups, 6);
					shader->Unbind();

					cubeMap->GenMipMap();
				};
			m_RP_GenCubemapFromSphericalMap = std::make_unique<RenderPassBase>();
			m_RP_GenCubemapFromSphericalMap->Setup(cfg);
		}

		// --- Precompute Irradiance Map Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Precompute Irradiance Map Pass";
			cfg.shaderPath = (shaderPathRoot / "IrradianceMap.glsl").string();
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* skyboxEnvMap = nullptr;
					_Texture* irrandianceMap = nullptr;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.Irradiance) {
								auto texCfg = TextureConfig::GetCubeMapTextureConfig("Irradiance");
								texCfg.Height = texCfg.Width = 64;
								iblComp.Irradiance = std::make_unique<_Texture>(texCfg);
								irrandianceMap = iblComp.Irradiance.get();
								skyboxEnvMap = iblComp.EnvTextureSkyBox.get();
							}
						}
					);
					if (!skyboxEnvMap) {
						return;
					}
					// Write to irrandiance Map
					irrandianceMap->BindTextureImage(0);

					// Read from skyboxEnvMap(cubemap)
					RenderCommand::BindTextureUnit(0, skyboxEnvMap->GetTextureID());
					
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_gCubeMap", 0);
					static int group = 16;
					uint32_t face = irrandianceMap->GetWidth();
					uint32_t numGroups = (face + group - 1) / group;
					shader->DispatchCompute(numGroups, numGroups, 6);
					shader->Unbind();
				};
			m_RP_PrecomputeIrradiance = std::make_unique<RenderPassBase>();
			m_RP_PrecomputeIrradiance->Setup(cfg);
		}

		// --- Prefilter Map Pass --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "Precompute Prefilter Map Pass";
			cfg.shaderPath = (shaderPathRoot / "Prefiltering.glsl").string();
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* skyboxEnvMap = nullptr;
					_Texture* prefilterMap = nullptr;
					int prefilterBaseRes = 512;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.Prefilter) {
								auto texCfg = TextureConfig::GetCubeMapTextureConfig("Prefilter");
								texCfg.GenMips = true;
								texCfg.Height = texCfg.Width = iblComp.PrefilterBaseRes;
								prefilterBaseRes = iblComp.PrefilterBaseRes;
								iblComp.Prefilter = std::make_unique<_Texture>(texCfg);
								prefilterMap = iblComp.Prefilter.get();
								skyboxEnvMap = iblComp.EnvTextureSkyBox.get();
							}
						}
					);
					if (!skyboxEnvMap) {
						return;
					}

					// Read from envMap(cubemap)
					RenderCommand::BindTextureUnit(0, skyboxEnvMap->GetTextureID());
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_gCubeMap", 0);

					static int group = 16;
					int maxMipLevels = int(log2(prefilterBaseRes)) + 1;
					for (int mip = 0; mip < maxMipLevels; mip++) {
						// Write to prefilter Map
						prefilterMap->BindTextureImage(0, mip);
						int mipSize = prefilterBaseRes >> mip;
						float roughness = (float)mip / (float)(maxMipLevels - 1);
						shader->SetFloat("u_Roughness", roughness);
						shader->SetFloat("u_Size", mipSize);
						shader->SetInt("u_MipLevel", mip);
						int numGroups = (mipSize + group - 1) / group;
						shader->DispatchCompute(numGroups, numGroups, 6);
					}

					shader->Unbind();
				};
			m_RP_Prefiltering = std::make_unique<RenderPassBase>();
			m_RP_Prefiltering->Setup(cfg);
		}

		// --- BRDF Integrate Pass --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "BRDF Integrate Pass";
			cfg.shaderPath = (shaderPathRoot / "BRDFIntegrate.glsl").string();
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* brdf = nullptr;
					bool Gen = false;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.BRDFLUT) {
								auto texCfg = TextureConfig::GetColorTextureConfig("BRDFLUT");
								texCfg.Width = texCfg.Height = 256;
								texCfg.InternalFmt = InternalFormat::RG16F;
								texCfg.DataFmt = DataFormat::RG; 
								texCfg.DataType = DataType::Float;
								iblComp.BRDFLUT = std::make_unique<_Texture>(texCfg);
								brdf = iblComp.BRDFLUT.get();
								Gen = true;
							}
						}
					);
					if (!Gen) {
						return;
					}

					brdf->BindTextureImage(0);  // Write to BRDF LUT
					auto shader = self.GetShader();
					shader->Bind();
					static int group = 16;
					int numGroups = (brdf->GetWidth() + group - 1) / group;
					shader->DispatchCompute(numGroups, numGroups, 6);
					shader->Unbind();
				};
			m_RP_BRDFLUT = std::make_unique<RenderPassBase>();
			m_RP_BRDFLUT->Setup(cfg);
		}
	}

	void _IBLPipeline::Execute() {
		m_RP_GenCubemapFromSphericalMap->Execute();
		m_RP_PrecomputeIrradiance->Execute();
		m_RP_Prefiltering->Execute();
		m_RP_BRDFLUT->Execute();
	}
}
