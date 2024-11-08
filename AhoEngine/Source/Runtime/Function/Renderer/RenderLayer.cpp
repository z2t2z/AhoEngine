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

		m_HDR = new OpenGLTexture2D((g_CurrentPath / "Asset" / "HDR" / "rogland_clear_night_4k.hdr").string()/*, true*/);
		m_HDR->SetTexType(TexType::HDR);
		g_CurrentPath = std::filesystem::current_path();
		TextureBuffer::Init();
		SetupUBO();
		SetupPrecomputeDiffuseIrradiancePipeline();
		m_Renderer->GetPipeline(RenderPipelineType::Precompute)->Execute();
		SetupRenderPipeline();
	}

	void RenderLayer::OnDetach() {
	}

	void RenderLayer::OnUpdate(float deltaTime) {
		m_Renderer->Render();
	}

	void RenderLayer::OnImGuiRender() {
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
		auto gBufferPass		 = SetupGBufferPass();
		//auto HiZPass			 = SetupHiZPass();
		auto ssaoPass			 = SetupSSAOPass();
		//auto ssrPass			 = SetupSSRPass();
		//auto blurRGBPass		 = SetupBlurRGBPass();
		auto blurPass			 = SetupBlurRPass();
		auto postProcessingPass  = SetupPostProcessingPass();
		auto drawSelectedPass	 = SetupDrawSelectedPass();
		auto FXAAPass			 = SetupFXAAPass();

		shadingPass->RegisterTextureBuffer({ shadowMapPass->GetTextureBuffer(TexType::Depth), TexType::Depth });
		shadingPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Position), TexType::Position });
		shadingPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Normal), TexType::Normal });
		shadingPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Albedo), TexType::Albedo });
		shadingPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::PBR), TexType::PBR });  // PBR param, metalic and roughness in rg channels respectively
		auto preComputePipeline = m_Renderer->GetPipeline(RenderPipelineType::Precompute);
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPass(RenderPassType::PrecomputeIrradiance)->GetTextureBuffer(TexType::Irradiance), TexType::Irradiance });
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPass(RenderPassType::GenLUT)->GetTextureBuffer(TexType::LUT), TexType::LUT });
		shadingPass->RegisterTextureBuffer({ preComputePipeline->GetRenderPass(RenderPassType::Prefilter)->GetTextureBuffer(TexType::Prefilter), TexType::Prefilter });
		shadingPass->RegisterTextureBuffer({ blurPass->GetTextureBuffer(TexType::Result), TexType::AO });

		//HiZPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Depth), TexType::Depth });

		FXAAPass->RegisterTextureBuffer({ postProcessingPass->GetTextureBuffer(TexType::Result), TexType::Result });


		ssaoPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Position), TexType::Position });
		ssaoPass->RegisterTextureBuffer({ gBufferPass->GetTextureBuffer(TexType::Normal), TexType::Normal });
		ssaoPass->RegisterTextureBuffer({ Utils::CreateNoiseTexture(32), TexType::Noise });
		blurPass->RegisterTextureBuffer({ ssaoPass->GetTextureBuffer(TexType::AO), TexType::Result });

		//ssrPass->RegisterTextureBuffer({ geoPassAttachments[0], "u_gPosition" });
		//ssrPass->RegisterTextureBuffer({ geoPassAttachments[1], "u_gNormal" });
		//ssrPass->RegisterTextureBuffer({ shadingPass->GetRenderTarget()->GetTextureAttachments().back(), "u_gAlbedo" });
		//ssrPass->RegisterTextureBuffer({ HiZPass->GetRenderTarget()->GetTextureAttachments().back(), "u_Depth" });

		//blurRGBPass->RegisterTextureBuffer({ ssrPass->GetRenderTarget()->GetTextureAttachments().back(), "u_TexToBlur" });

		postProcessingPass->RegisterTextureBuffer({ shadingPass->GetTextureBuffer(TexType::Result), TexType::Result });
		postProcessingPass->RegisterTextureBuffer({ drawSelectedPass->GetTextureBuffer(TexType::Result), TexType::Entity });

		/* This is order dependent! */
		pipeline->RegisterRenderPass(std::move(shadowMapPass), RenderDataType::SceneData);
		pipeline->RegisterRenderPass(std::move(gBufferPass), RenderDataType::SceneData);
		//pipeline->RegisterRenderPass(std::move(HiZPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(ssaoPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(blurPass), RenderDataType::ScreenQuad);
		//pipeline->RegisterRenderPass(std::move(ssrPass), RenderDataType::ScreenQuad);
		//pipeline->RegisterRenderPass(std::move(blurRGBPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(shadingPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(drawSelectedPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(postProcessingPass), RenderDataType::ScreenQuad);
		pipeline->RegisterRenderPass(std::move(FXAAPass), RenderDataType::ScreenQuad);
		m_Renderer->SetCurrentRenderPipeline(pipeline);
		RenderCommand::SetDepthTest(true);
	}

	void RenderLayer::SetupPrecomputeDiffuseIrradiancePipeline() {
		RenderPipeline* pipeline = new RenderPipeline();
		auto genCubeMapPass = SetupGenCubemapFromHDRPass();
		auto preComputePass = SetupPrecomputeIrradiancePass();
		auto prefilterPass = SetupPrefilteredPass();
		auto genLUTPass = SetupGenLUTPass();

		genCubeMapPass->RegisterTextureBuffer({ m_HDR, TexType::HDR });
		preComputePass->RegisterTextureBuffer({ genCubeMapPass->GetTextureBuffer(TexType::CubeMap), TexType::CubeMap });
		prefilterPass->RegisterTextureBuffer ({ genCubeMapPass->GetTextureBuffer(TexType::CubeMap), TexType::CubeMap });

		pipeline->RegisterRenderPass(std::move(genCubeMapPass), RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(preComputePass), RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(prefilterPass),  RenderDataType::UnitCube);
		pipeline->RegisterRenderPass(std::move(genLUTPass),		RenderDataType::UnitCube);

		pipeline->SetType(RenderPipelineType::Precompute);
		m_Renderer->AddRenderPipeline(pipeline);
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupFXAAPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pp = Shader::Create(g_CurrentPath / "ShaderSrc" / "FXAA.glsl");
		pass->SetShader(pp);
		TexSpec texSpecColor; texSpecColor.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::FXAA);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupPrefilteredPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;
		TexSpec spec;
		spec.target = TexTarget::TextureCubemap; spec.width = spec.height = 128;
		spec.internalFormat = TexInterFormat::RGB16F; spec.dataFormat = TexDataFormat::RGB; spec.dataType = TexDataType::Float;
		spec.filterModeMin = TexFilterMode::LinearMipmapLinear;
		spec.mipLevels = 5;
		spec.type = TexType::Prefilter;
		FBSpec fbSpec(128, 128, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::Prefilter);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupGenLUTPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		spec.internalFormat = TexInterFormat::RG16F; spec.dataFormat = TexDataFormat::RG; spec.dataType = TexDataType::Float; spec.type = TexType::LUT;
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
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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

	std::unique_ptr<RenderPass> RenderLayer::SetupPrecomputeIrradiancePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		FBSpec fbSpec(32, 32, { spec, depth });
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::PrecomputeIrradiance);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupGBufferPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
			for (const auto& data : renderData) {
				if (!data->ShouldBeRendered()) {
					continue;
				}
				data->Bind(shader);

				shader->SetUint("u_EntityID", data->GetEntityID());
				if (GlobalState::g_SelectedEntityID == data->GetEntityID()) {
					GlobalState::g_SelectedData = data;
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
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		auto shader = Shader::Create((g_CurrentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth;

		TexSpec positionAttachment;
		positionAttachment.internalFormat = TexInterFormat::RGBA16F;
		positionAttachment.dataFormat = TexDataFormat::RGBA;
		positionAttachment.dataType = TexDataType::Float;
		positionAttachment.filterModeMin = TexFilterMode::Nearest;
		positionAttachment.filterModeMag = TexFilterMode::Nearest;
		positionAttachment.type = TexType::Position;

		TexSpec normalAttachment = positionAttachment;
		normalAttachment.wrapModeS = TexWrapMode::None;
		normalAttachment.type = TexType::Normal;

		TexSpec albedoAttachment;
		albedoAttachment.type = TexType::Albedo;

		TexSpec entityAttachment;
		entityAttachment.internalFormat = TexInterFormat::UINT;
		entityAttachment.dataFormat = TexDataFormat::UINT;
		entityAttachment.dataType = TexDataType::UnsignedInt;
		entityAttachment.type = TexType::Entity;

		TexSpec pbrAttachment = positionAttachment;
		pbrAttachment.internalFormat = TexInterFormat::RGBA16F; pbrAttachment.dataFormat = TexDataFormat::RGBA; // Don't use rbga8 cause it will be normalized!
		pbrAttachment.dataType = TexDataType::Float;
		pbrAttachment.type = TexType::PBR;

		FBSpec fbSpec(1280u, 720u, { positionAttachment, normalAttachment, albedoAttachment, pbrAttachment, entityAttachment, depth });

		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::SSAOGeo);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupSSAOPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
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

	std::unique_ptr<RenderPass> RenderLayer::SetupBlurRPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
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

	std::unique_ptr<RenderPass> RenderLayer::SetupBlurRGBPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		std::string FileName = (g_CurrentPath / "ShaderSrc" / "BlurRGB.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
		spec.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { spec });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::BlurRGB);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupShadingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

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
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec spec;
		spec.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { spec });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::Shading);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupDrawSelectedPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			if (GlobalState::g_IsEntityIDValid && GlobalState::g_SelectedData) {
				shader->Bind();
				renderTarget->EnableAttachments(0);
				RenderCommand::Clear(ClearFlags::Color_Buffer);
				const auto& data = GlobalState::g_SelectedData;
				shader->SetUint("u_EntityID", data->GetEntityID());
				data->Bind(shader);
				data->IsInstanced() ? RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()) : RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
				renderTarget->Unbind();
				shader->Unbind();
			}
			});
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>();
		pass->SetRenderCommand(std::move(cmdBuffer));

		auto pickingShader = Shader::Create(g_CurrentPath / "ShaderSrc" / "DrawSelected.glsl");
		pass->SetShader(pickingShader);
		TexSpec spec;
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

	std::unique_ptr<RenderPass> RenderLayer::SetupSSRPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
				texBuffer.m_Texture->Bind(texOffset++);
			}

			int mipLevel = renderTarget->GetTextureAttachments().back()->GetSpecification().mipLevels;
			shader->SetInt("u_Width", renderTarget->GetSpecification().Width);
			shader->SetInt("u_Height", renderTarget->GetSpecification().Height);
			shader->SetInt("u_MipLevelMax", mipLevel);
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
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->Bind();
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
				texBuffer.m_Texture->Bind(texOffset++);
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
		FBSpec fbSpec(1280u, 720u, { });
		auto TexO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(TexO);
		pass->SetRenderPassType(RenderPassType::HiZ);
		return pass;
	}

	// Draw object outlines, lights, skeletons, etc.
	std::unique_ptr<RenderPass> RenderLayer::SetupPostProcessingPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);

			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
				texBuffer.m_Texture->Bind(texOffset++);
			}

			shader->SetUint("u_SelectedEntityID", GlobalState::g_SelectedEntityID);
			shader->SetBool("u_IsEntityIDValid", GlobalState::g_IsEntityIDValid);

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
		texSpecColor.type = TexType::Result;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::PostProcessing);
		return pass;
	}

	std::unique_ptr<RenderPass> RenderLayer::SetupShadowMapPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth32; depth.dataFormat = TexDataFormat::DepthComponent; 
		depth.type = TexType::Depth;
		FBSpec fbSpec(2048, 2048, { depth });  // Hardcode fow now
		auto FBO = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(FBO);
		pass->SetRenderPassType(RenderPassType::Depth);
		return pass;
	}
}