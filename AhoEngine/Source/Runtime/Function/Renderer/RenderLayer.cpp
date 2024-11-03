#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"
#include <imgui.h>

namespace Aho {
	static std::filesystem::path g_CurrentPath;

	static glm::mat4 g_Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	static glm::mat4 g_Views[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	RenderLayer::RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("RenderLayer"), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void RenderLayer::OnAttach() {
		AHO_CORE_INFO("RenderLayer on attach");
		m_HDR = new OpenGLTexture2D((g_CurrentPath / "Asset" / "HDR" / "meadow_4k.hdr").string()/*, true*/);
		g_CurrentPath = std::filesystem::current_path();

		SetupPrecomputeDiffuseIrradiancePipeline();
		m_Renderer->GetPipeline(RenderPipelineType::Precompute)->Execute();
		SetupRenderPipeline();
		SetupUBO();
	}

	void RenderLayer::OnDetach() {
	}

	void RenderLayer::OnUpdate(float deltaTime) {
		m_Renderer->Render();
	}

	void RenderLayer::OnImGuiRender() {
		/* In game UI logic, HUD or something */
	}

	void RenderLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::UploadRenderData) {
			e.SetHandled();
			AHO_CORE_WARN("Recieving a UploadRenderDataEvent!");
			const auto& pipeLine = m_Renderer->GetPipeline(RenderPipelineType::Default);
			for (const auto& data : ((UploadRenderDataEvent*)&e)->GetRawData()) {
				pipeLine->AddRenderData(data);
			}
		}
	}

	void RenderLayer::SetupUBO() {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<LightUBO>(1);
		UBOManager::RegisterUBO<RandomKernelUBO>(2); RandomKernelUBO rndUBO; UBOManager::UpdateUBOData(2, rndUBO);
		UBOManager::RegisterUBO<AnimationUBO>(3);
		UBOManager::RegisterUBO<SkeletonUBO>(4);
	}

	// TDOO: Should have an easier way, like binding the name into texture so that no need to hardcode it
	void RenderLayer::SetupRenderPipeline() {
		RenderPipeline* pipeline = new RenderPipeline();
		auto shadingPass         = SetupShadingPass();
		auto shadowMapPass		 = SetupShadowMapPass();
		auto debugPass			 = SetupDebugPass();
		auto gBufferPass		 = SetupGBufferPass();
		auto HiZPass			 = SetupHiZPass();
		auto ssaoPass			 = SetupSSAOPass();
		auto ssrPass			 = SetupSSRPass();
		auto blurRGBPass		 = SetupBlurPass();
		auto blurPass			 = SetupSSAOBlurPass();
		auto postProcessingPass  = SetupPostProcessingPass();

		shadingPass->RegisterTextureBuffer({ shadowMapPass->GetRenderTarget()->GetDepthTexture(), "u_DepthMap" });
		auto geoPassAttachments = gBufferPass->GetRenderTarget()->GetTextureAttachments();

		/* Debug pass use the same fbo as gbufferPass */
		debugPass->SetRenderTarget(gBufferPass->GetRenderTarget());

		HiZPass->RegisterTextureBuffer({ gBufferPass->GetRenderTarget()->GetTextureAttachment(3), "u_Depth" }); // this is the color attachement that contains depth info in a Hi-Z structure
		HiZPass->RegisterTextureBuffer({ HiZPass->GetRenderTarget()->GetTextureAttachments().back(), "u_SelfDepth"});

		ssaoPass->RegisterTextureBuffer({ geoPassAttachments[0], "u_gPosition" }); 
		ssaoPass->RegisterTextureBuffer({ geoPassAttachments[1], "u_gNormal" }); 
		ssaoPass->RegisterTextureBuffer({ Utils::CreateNoiseTexture(32), "u_Noise" }); 

		ssrPass->RegisterTextureBuffer({ geoPassAttachments[0], "u_gPosition" });
		ssrPass->RegisterTextureBuffer({ geoPassAttachments[1], "u_gNormal" });
		ssrPass->RegisterTextureBuffer({ shadingPass->GetRenderTarget()->GetTextureAttachments().back(), "u_gAlbedo" });
		ssrPass->RegisterTextureBuffer({ HiZPass->GetRenderTarget()->GetTextureAttachments().back(), "u_Depth" });

		shadingPass->RegisterTextureBuffer({ geoPassAttachments[0], "u_gPosition" });
		shadingPass->RegisterTextureBuffer({ geoPassAttachments[1], "u_gNormal" });
		shadingPass->RegisterTextureBuffer({ geoPassAttachments[2], "u_gAlbedo" });

		blurRGBPass->RegisterTextureBuffer({ ssrPass->GetRenderTarget()->GetTextureAttachments().back(), "u_TexToBlur" });
		blurPass->RegisterTextureBuffer({ ssaoPass->GetRenderTarget()->GetTextureAttachments().back(), "u_SSAO" });
		
		auto preComputePipeline = m_Renderer->GetPipeline(RenderPipelineType::Precompute);
		shadingPass->RegisterTextureBuffer({ blurPass->GetRenderTarget()->GetTextureAttachments().back(), "u_SSAO" });
		shadingPass->RegisterTextureBuffer({ blurRGBPass->GetRenderTarget()->GetTextureAttachments().back(), "u_Specular"});
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPassTarget(RenderPassType::PrecomputeIrradiance)->GetTextureAttachments().back(), "u_Irradiance"}); 
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPassTarget(RenderPassType::Prefilter)->GetTextureAttachments().back(), "u_Prefilter" });			
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPassTarget(RenderPassType::GenLUT)->GetTextureAttachments().back(), "u_LUT"});				
		shadingPass->RegisterTextureBuffer({ geoPassAttachments[4], "u_PBR" });  // PBR param, metalic and roughness in rg channels respectively

		postProcessingPass->RegisterTextureBuffer({ shadingPass->GetRenderTarget()->GetTextureAttachments().back(), "u_Result" });
		postProcessingPass->RegisterTextureBuffer({ gBufferPass->GetRenderTarget()->GetTextureAttachment(4), "u_gEntity"});
		postProcessingPass->RegisterTextureBuffer({ debugPass->GetRenderTarget()->GetTextureAttachments().back(), "u_Debug" }); // DebugAttachment!
		postProcessingPass->RegisterTextureBuffer({ gBufferPass->GetRenderTarget()->GetTextureAttachment(3), "u_DepthMap" });

		/* This is order dependent! */
		pipeline->RegisterRenderPass(std::move(shadowMapPass), RenderDataType::SceneData);
		pipeline->RegisterRenderPass(std::move(gBufferPass), RenderDataType::SceneData);
		pipeline->RegisterRenderPass(std::move(debugPass), RenderDataType::DebugData);
		pipeline->RegisterRenderPass(std::move(HiZPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(ssaoPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(blurPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(ssrPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(blurRGBPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(shadingPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(postProcessingPass), RenderDataType::ScreenQuad);

		m_Renderer->SetCurrentRenderPipeline(pipeline);
		RenderCommand::SetDepthTest(true);
	}

	void RenderLayer::SetupPrecomputeDiffuseIrradiancePipeline() {
		RenderPipeline* pipeline = new RenderPipeline();
		auto genCubeMapPass = SetupGenCubemapFromHDRPass();
		auto preComputePass = SetupPrecomputeIrradiancePass();
		auto prefilterPass = SetupPrefilteredPass();
		auto genLUTPass = SetupGenLUTPass();

		genCubeMapPass->RegisterTextureBuffer({ m_HDR, "u_HDR"});
		preComputePass->RegisterTextureBuffer({ genCubeMapPass->GetRenderTarget()->GetTextureAttachments().back(), "u_CubeMap" });
		prefilterPass->RegisterTextureBuffer({ genCubeMapPass->GetRenderTarget()->GetTextureAttachments().back(), "u_CubeMap" });

		pipeline->RegisterRenderPass(std::move(genCubeMapPass), RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(preComputePass), RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(prefilterPass), RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(genLUTPass), RenderDataType::UnitCube);

		pipeline->SetType(RenderPipelineType::Precompute);
		m_Renderer->AddRenderPipeline(pipeline);
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupPrefilteredPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			shader->Bind();

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			shader->SetMat4("u_Projection", g_Projection);
			renderTarget->Bind();

			uint32_t maxMipLevel = 5;
			for (uint32_t i = 0; i < maxMipLevel; i++) {
				uint32_t width = static_cast<uint32_t>(128 * std::pow(0.5, i));
				uint32_t height = static_cast<uint32_t>(128 * std::pow(0.5, i));
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
		TexSpec spec; TexSpec depth; depth.dataFormat = TexDataFormat::DepthComponent;
		spec.target = TexTarget::TextureCubemap;
		spec.width = spec.height = 128;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		spec.filterModeMin = TexFilterMode::LinearMipmapLinear;
		spec.mipLevels = 5;
		FBSpec fbSpec(128, 128, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::Prefilter);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupGenLUTPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		TexSpec spec; TexSpec depth; depth.dataFormat = TexDataFormat::DepthComponent;
		spec.width = spec.height = 512;
		spec.internalFormat = TexInterFormat::RG16F; spec.dataFormat = TexDataFormat::RG; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(512, 512, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::GenLUT);
		return pass;
	}

	// TODO: Hardcode a lot to suit cubemap
	std::unique_ptr<RenderPass> RenderLayer::SetupGenCubemapFromHDRPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
			shader->Bind();

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
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
		TexSpec depth; depth.dataFormat = TexDataFormat::DepthComponent;
		TexSpec spec;
		spec.target = TexTarget::TextureCubemap;
		spec.width = spec.height = 512;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(512, 512, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::GenCubemap);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupPrecomputeIrradiancePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
			shader->Bind();

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			shader->SetMat4("u_Projection", g_Projection);
			renderTarget->Bind();
			for (int i = 0; i < 6; i++) {
				RenderCommand::Clear(ClearFlags::Depth_Buffer);
				shader->SetMat4("u_View", g_Views[i]);
				renderTarget->BindCubeMap(renderTarget->GetTextureAttachments()[0], i);  // This is the cubemap we are going to write the calculated irradiance
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
		const auto shader = Shader::Create(g_CurrentPath / "ShaderSrc" / "IrradianceConv.glsl");
		pass->SetShader(shader);

		TexSpec depth; depth.dataFormat = TexDataFormat::DepthComponent;
		TexSpec spec;
		spec.target = TexTarget::TextureCubemap;
		spec.width = spec.height = 32;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		FBSpec fbSpec(32, 32, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::PrecomputeIrradiance);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupDebugPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(5, 2);
			RenderCommand::SetClearColor(glm::vec4(0.0f));
			RenderCommand::Clear(ClearFlags::Depth_Buffer);
			RenderCommand::SetClearColor(RenderCommand::s_DefaultClearColor);
			for (const auto& data : renderData) {
				if (!data->ShouldBeRendered()) {
					continue;
				}
				data->Bind(shader);
				if (data->IsInstanced()) {
					shader->SetBool("u_IsInstanced", true);
					RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()); // Draw skeleton using lines
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
		pass->SetRenderCommand(std::move(cmdBuffer));
		const auto debugShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "Debug.glsl");
		pass->SetShader(debugShader);
		pass->SetRenderPassType(RenderPassType::Debug);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupGBufferPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
			for (const auto& data : renderData) {
				if (!data->ShouldBeRendered()) {
					continue;
				}
				data->Bind(shader);
				if (data->IsInstanced()) {
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
		auto shader = Shader::Create((g_CurrentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec positionAttachment, depthAttachment, depthComponent;
		depthComponent.dataFormat = TexDataFormat::DepthComponent;
		positionAttachment.internalFormat = TexInterFormat::RGBA16F;
		positionAttachment.dataFormat = TexDataFormat::RGBA;
		positionAttachment.dataType = TexDataType::Float;
		positionAttachment.filterModeMin = TexFilterMode::Nearest;
		positionAttachment.filterModeMag = TexFilterMode::Nearest;
		TexSpec normalAttachment = positionAttachment;

		depthAttachment = positionAttachment;
		depthAttachment.internalFormat = TexInterFormat::RED32F;
		depthAttachment.dataFormat = TexDataFormat::RED;
		auto depthLightAttachment = depthAttachment;
		normalAttachment.wrapModeS = TexWrapMode::None;
		TexSpec albedoAttachment, debugAttachment;
		auto entityAttachment = depthAttachment;
		entityAttachment.internalFormat = TexInterFormat::UINT;
		entityAttachment.dataFormat = TexDataFormat::UINT;
		entityAttachment.dataType = TexDataType::UnsignedInt;

		auto pbrAttachment = positionAttachment;
		pbrAttachment.internalFormat = TexInterFormat::RGB8; pbrAttachment.dataFormat = TexDataFormat::RGB;
		FBSpec fbSpec(1280u, 720u, { positionAttachment, normalAttachment, albedoAttachment, depthAttachment, pbrAttachment, entityAttachment, debugAttachment, depthComponent });

		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::SSAOGeo);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupSSAOPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec ssaoColor;
		{
			ssaoColor.internalFormat = TexInterFormat::RED;
			ssaoColor.dataFormat = TexDataFormat::RED;
			ssaoColor.dataType = TexDataType::Float;
			ssaoColor.filterModeMin = TexFilterMode::Nearest;
			ssaoColor.filterModeMag = TexFilterMode::Nearest;
		}
		FBSpec fbSpec(1280u, 720u, { ssaoColor });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::SSAO);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupSSAOBlurPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::string FileName = (g_CurrentPath / "ShaderSrc" / "Blur.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec texSpecColor;
		{
			texSpecColor.internalFormat = TexInterFormat::RED;
			texSpecColor.dataFormat = TexDataFormat::RED;
			texSpecColor.dataType = TexDataType::Float;
		}
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::BlurR);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupBlurPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::string FileName = (g_CurrentPath / "ShaderSrc" / "BlurColor.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec texSpecColor;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::BlurRGB);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupShadingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec texSpecColor;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::Shading);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupPickingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			for (const auto& data : renderData) {
				data->Bind(shader);
				data->IsInstanced() ? RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()) : RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			});
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pickingShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "MousePicking.glsl");
		pass->SetShader(pickingShader);
		TexSpec texSpecColor;
		TexSpec texSpecDepth; texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		FBSpec fbSpec(1280u, 720u, { texSpecDepth, texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::Pick);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupSSRPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			int mipLevel = renderTarget->GetTextureAttachments().back()->GetSpecification().mipLevels;
			shader->SetInt("u_Width", renderTarget->GetSpecification().Width);
			shader->SetInt("u_Height", renderTarget->GetSpecification().Height);
			shader->SetInt("u_MipLevelMax", mipLevel);
			shader->SetInt("u_gPosition", 0);
			shader->SetInt("u_gNormal", 1);
			shader->SetInt("u_gAlbedo", 2);
			shader->SetInt("u_Depth", 3);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});

		std::string FileName = (g_CurrentPath / "ShaderSrc" / "SSR.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec texSpecColor;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::SSRvs);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupHiZPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			int height = renderTarget->GetTextureAttachments().back()->GetSpecification().height;
			int width = renderTarget->GetTextureAttachments().back()->GetSpecification().width;
			int mipLevel = renderTarget->GetTextureAttachments().back()->GetSpecification().mipLevels;
			for (int i = 0; i < mipLevel; i++) {
				shader->SetInt("u_MipmapLevels", i);
				RenderCommand::SetViewport(std::max(1, width >> i), std::max(height >> i, 1));
				RenderCommand::BindRenderTarget(0, renderTarget->GetTextureAttachments().back()->GetTextureID(), i);
				RenderCommand::Clear(ClearFlags::Color_Buffer);
				for (const auto& data : renderData) {
					data->Bind(shader);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto depthShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "HiZ.glsl");
		pass->SetShader(depthShader);
		TexSpec depthAttachment;
		depthAttachment.target = TexTarget::Texture2D;
		depthAttachment.internalFormat = TexInterFormat::RED32F;
		depthAttachment.dataFormat = TexDataFormat::RED;
		depthAttachment.filterModeMin = TexFilterMode::NearestMipmapNearest;
		depthAttachment.filterModeMag = TexFilterMode::NearestMipmapNearest;
		depthAttachment.mipLevels = 1;
		FBSpec fbSpec(1280u, 720u, { depthAttachment });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::HiZ);
		return pass;
	}

	// Draw object outlines, lights, skeletons, etc.
	std::unique_ptr<RenderPass> RenderLayer::SetupPostProcessingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(texBuffer.name, texOffset);
				texBuffer.tex->Bind(texOffset++);
			}

			shader->SetUint("u_SelectedEntityID", GlobalState::g_SelectedEntityID);
			shader->SetBool("u_IsEntityIDValid", GlobalState::g_IsEntityIDValid);
			shader->SetInt("u_DrawDebug", GlobalState::g_ShowDebug);

			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pp = Shader::Create(g_CurrentPath / "ShaderSrc" / "Postprocessing.glsl");
		pass->SetShader(pp);
		TexSpec texSpecColor;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::PostProcessing);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupShadowMapPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffers>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));
		
		auto depthShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "ShadowMap.glsl");
		pass->SetShader(depthShader);
		TexSpec texSpecDepth;
		texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		texSpecDepth.internalFormat = TexInterFormat::Depth24;	// ......
		texSpecDepth.dataType = TexDataType::Float;
		texSpecDepth.wrapModeS = TexWrapMode::Repeat;
		texSpecDepth.wrapModeT = TexWrapMode::Repeat;
		texSpecDepth.filterModeMin = TexFilterMode::Nearest;
		texSpecDepth.filterModeMag = TexFilterMode::Nearest;
		FBSpec fbSpec(2048, 2048, { texSpecDepth });  // Hardcode fow now
		auto depthTexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(depthTexO);
		pass->SetRenderPassType(RenderPassType::Depth);
		return pass;
	}
}