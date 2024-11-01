#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"
#include <imgui.h>

namespace Aho {
	RenderLayer::RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("RenderLayer"), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void RenderLayer::OnAttach() {
		AHO_CORE_INFO("RenderLayer on attach");
		SetupForwardRenderPipeline();
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
		/* Parameters on changed event */
		if (e.GetEventType() == EventType::UploadRenderData) {
			e.SetHandled();
			AHO_CORE_WARN("Recieving a UploadRenderDataEvent!");
			for (const auto& pipeLine : *m_Renderer) {
				for (const auto& data : ((UploadRenderDataEvent*)&e)->GetRawData()) {
					if (data->IsLine()) {
						pipeLine->AddLineRenderData(data);
					}
					else {
						pipeLine->AddRenderData(data); // TODO: Memory wastage, should be optimized
					}
				}
			}
		}
	}

	void RenderLayer::SetupForwardRenderPipeline() {
		RenderPipelineDefault* pipeline = new RenderPipelineDefault();
		auto shadingPass = SetupShadingPass();
		auto shadowMapPass = SetupShadowMapPass();
		auto debugPass = SetupDebugPass();
		shadingPass->AddGBuffer(shadowMapPass->GetRenderTarget()->GetDepthTexture());

		// Setup SSAO
		auto gBufferPass = SetupGBufferPass();
		auto HiZPass = SetupHiZPass();
		auto geoPassAttachments = gBufferPass->GetRenderTarget()->GetTextureAttachments();

		/* Debug pass use the same TexO as gbufferPass */
		debugPass->SetRenderTarget(gBufferPass->GetRenderTarget());

		auto ssaoPass = SetupSSAOPass();
		auto ssrPass = SetupSSRPass();
		HiZPass->AddGBuffer(gBufferPass->GetRenderTarget()->GetTextureAttachment(3)); // this is the color attachement that contains depth info in a Hi-Z structure
		HiZPass->AddGBuffer(HiZPass->GetRenderTarget()->GetTextureAttachments().back());
		// Note that order matters!
		for (int i = 0; i < 3; i++) {
			if (i < 2) {
				ssaoPass->AddGBuffer(geoPassAttachments[i]); // gPostion, gNormal
				ssrPass->AddGBuffer(geoPassAttachments[i]); // gPostion, gNormal
			}
			shadingPass->AddGBuffer(geoPassAttachments[i]);     // gPostion, gNormal, gAlbedo
		}
		ssaoPass->AddGBuffer(Utils::CreateNoiseTexture(32)); // ssaoPass: gPostion, gNormal, gNoise
		ssrPass->AddGBuffer(shadingPass->GetRenderTarget()->GetTextureAttachments().back()); // SSR based on the rendered image

		ssrPass->AddGBuffer(HiZPass->GetRenderTarget()->GetTextureAttachments().back()); // Depthmap, from view space

		auto blurPass = SetupSSAOBlurPass();
		blurPass->AddGBuffer(ssaoPass->GetRenderTarget()->GetTextureAttachments().back());
		shadingPass->AddGBuffer(blurPass->GetRenderTarget()->GetTextureAttachments().back());     // SSAO

		auto postProcessingPass = SetupPostProcessingPass();
		postProcessingPass->AddGBuffer(shadingPass->GetRenderTarget()->GetTextureAttachments().back());
		postProcessingPass->AddGBuffer(gBufferPass->GetRenderTarget()->GetTextureAttachment(4));
		postProcessingPass->AddGBuffer(debugPass->GetRenderTarget()->GetTextureAttachments().back()); // DebugAttachment!

		pipeline->AddRenderPass(shadingPass);
		pipeline->AddRenderPass(shadowMapPass);
		pipeline->AddRenderPass(debugPass);
		pipeline->AddRenderPass(SetupPickingPass());
		pipeline->AddRenderPass(blurPass);
		pipeline->AddRenderPass(gBufferPass);
		pipeline->AddRenderPass(ssaoPass);
		pipeline->AddRenderPass(ssrPass);
		pipeline->AddRenderPass(HiZPass);
		pipeline->AddRenderPass(postProcessingPass);
		pipeline->SortRenderPasses();
		m_Renderer->SetCurrentRenderPipeline(pipeline);
	}

	void RenderLayer::SetupUBO() {
		UBOManager::RegisterUBO<CameraUBO>(0);
		UBOManager::RegisterUBO<LightUBO>(1);
		UBOManager::RegisterUBO<RandomKernelUBO>(2); RandomKernelUBO rndUBO; UBOManager::UpdateUBOData(2, rndUBO);
		UBOManager::RegisterUBO<AnimationUBO>(3);
		UBOManager::RegisterUBO<SkeletonUBO>(4);
	}

	RenderPass* RenderLayer::SetupDebugPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		RenderPassDefault* debugPass = new RenderPassDefault();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(4, 2);
			RenderCommand::SetClearColor(glm::vec4(0.0f));
			RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
			RenderCommand::SetClearColor(RenderCommand::s_DefaultClearColor);
			for (const auto& data : renderData) {
				if (!data->ShouldBeRendered()) {
					continue;
				}
				data->Bind(shader);
				if (data->IsInstanced()) {
					shader->SetBool("u_IsInstanced", true);
					RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()); // Draw skeleton using lines
				}
				else {
					shader->SetBool("u_IsInstanced", false);
					RenderCommand::DrawIndexed(data->GetVAO());
				}
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
		});
		debugPass->SetRenderCommand(cmdBuffer);
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		const auto debugShader = Shader::Create(currentPath / "ShaderSrc" / "Debug.glsl");
		debugPass->SetShader(debugShader);
		debugPass->SetRenderPassType(RenderPassType::Debug);
		return debugPass;
	}

	RenderPass* RenderLayer::SetupGBufferPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0, 6);
			RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
			for (const auto& data : renderData) {
				if (!data->ShouldBeRendered()) {
					continue;
				}
				data->Bind(shader);
				if (data->IsInstanced()) {
					shader->SetBool("u_IsInstanced", true);
					RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount());
				}
				else {
					shader->SetBool("u_IsInstanced", false);
					RenderCommand::DrawIndexed(data->GetVAO());
				}
				data->Unbind();
			}
			//renderTarget->Unbind();
			//shader->Unbind();
		});
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		auto shader = Shader::Create((currentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		RenderPassDefault* renderPass = new RenderPassDefault();
		AHO_CORE_ASSERT(shader->IsCompiled());
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec positionAttachment, depthAttachment, depthComponent;
		depthComponent.dataFormat = TexDataFormat::DepthComponent;
		positionAttachment.internalFormat = TexInterFormat::RGBA16F;
		positionAttachment.dataFormat = TexDataFormat::RGBA;
		positionAttachment.dataType = TexDataType::Float;
		positionAttachment.target = TexTarget::Texture2D;
		positionAttachment.wrapModeS = TexWrapMode::Clamp;
		positionAttachment.wrapModeT = TexWrapMode::Clamp;
		positionAttachment.filterModeMin = TexFilterMode::Nearest;
		positionAttachment.filterModeMag = TexFilterMode::Nearest;
		TexSpec normalAttachment = positionAttachment;

		depthAttachment = positionAttachment;
		depthAttachment.internalFormat = TexInterFormat::RED32F;
		depthAttachment.dataFormat = TexDataFormat::RED;
		auto depthLightAttachment = depthAttachment;
		normalAttachment.wrapModeS = TexWrapMode::None;
		TexSpec albedoAttachment = normalAttachment;
		albedoAttachment.dataType = TexDataType::UnsignedByte;
		albedoAttachment.internalFormat = TexInterFormat::RGBA8;
		auto entityAttachment = depthAttachment;
		entityAttachment.internalFormat = TexInterFormat::UINT;
		entityAttachment.dataFormat = TexDataFormat::UINT;
		entityAttachment.dataType = TexDataType::UnsignedInt;
		auto debugAttachment = albedoAttachment;
		FBSpec fbSpec(1280u, 720u, { positionAttachment, normalAttachment, albedoAttachment, depthAttachment, entityAttachment, debugAttachment, depthComponent });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::SSAOGeo);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++);
			}
			shader->SetInt("u_gPosition", 0);
			shader->SetInt("u_gNormal", 1);
			shader->SetInt("u_Noise", 2);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		auto shader = Shader::Create((currentPath / "ShaderSrc" / "SSAO.glsl").string());
		RenderPassDefault* renderPass = new RenderPassDefault();
		AHO_CORE_ASSERT(shader->IsCompiled());
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec ssaoColor;
		TexSpec texSpecDepth; texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		{
			ssaoColor.internalFormat = TexInterFormat::RED;
			ssaoColor.dataFormat = TexDataFormat::RED;
			ssaoColor.dataType = TexDataType::Float;
			ssaoColor.target = TexTarget::Texture2D;
			ssaoColor.filterModeMin = TexFilterMode::Nearest;
			ssaoColor.filterModeMag = TexFilterMode::Nearest;
		}
		FBSpec fbSpec(1280u, 720u, { ssaoColor });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::SSAO);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOBlurPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
			}
			shader->SetInt("u_SSAO", 0);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "Blur.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec texSpecColor;
		{
			texSpecColor.internalFormat = TexInterFormat::RED;
			texSpecColor.dataFormat = TexDataFormat::RED;
			texSpecColor.dataType = TexDataType::Float;
			texSpecColor.target = TexTarget::Texture2D;
			texSpecColor.wrapModeS = TexWrapMode::Clamp;
			texSpecColor.wrapModeT = TexWrapMode::Clamp;
			texSpecColor.filterModeMin = TexFilterMode::Nearest;
			texSpecColor.filterModeMag = TexFilterMode::Nearest;
		}
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::Blur);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOLightingPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
			}
			shader->SetInt("u_gPosition", 0);
			shader->SetInt("u_gNormal", 1);
			shader->SetInt("u_gAlbedo", 2);
			shader->SetInt("u_gSSAO", 3);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "SSAO_LightingPass.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec texSpecColor;
		TexSpec texSpecDepth; texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		{
			texSpecColor.internalFormat = TexInterFormat::RGBA8;
			texSpecColor.dataFormat = TexDataFormat::RGBA;
			texSpecColor.dataType = TexDataType::UnsignedByte;
			texSpecColor.target = TexTarget::Texture2D;
			texSpecColor.wrapModeS = TexWrapMode::Clamp;
			texSpecColor.wrapModeT = TexWrapMode::Clamp;
			texSpecColor.filterModeMin = TexFilterMode::Nearest;
			texSpecColor.filterModeMag = TexFilterMode::Nearest;
		}
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::SSAOLighting);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupShadingPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // TODO;
			}
			shader->SetInt("u_DepthMap", 0);
			shader->SetInt("u_gPosition", 1);
			shader->SetInt("u_gNormal", 2);
			shader->SetInt("u_gAlbedo", 3);
			shader->SetInt("u_SSAO", 4);
			for (const auto& data : renderData) {
				//data-> TODO: no needs to bind material uniforms
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "pbrShader.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec texSpecColor;
		TexSpec texSpecDepth; texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		texSpecColor.internalFormat = TexInterFormat::RGBA8;
		texSpecColor.dataFormat = TexDataFormat::RGBA;
		texSpecColor.dataType = TexDataType::UnsignedByte;
		texSpecColor.target = TexTarget::Texture2D;
		texSpecColor.wrapModeS = TexWrapMode::Clamp;
		texSpecColor.wrapModeT = TexWrapMode::Clamp;
		texSpecColor.filterModeMin = TexFilterMode::Nearest;
		texSpecColor.filterModeMag = TexFilterMode::Nearest;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::Shading);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupPickingPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			for (const auto& data : renderData) {
				data->Bind(shader);
				data->IsInstanced() ? RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount()) : RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}

			});
		RenderPassDefault* pickingPass = new RenderPassDefault();
		pickingPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto pickingShader = Shader::Create(currentPath / "ShaderSrc" / "MousePicking.glsl");
		pickingPass->SetShader(pickingShader);
		TexSpec texSpecColor;
		TexSpec texSpecDepth; texSpecDepth.dataFormat = TexDataFormat::DepthComponent;
		texSpecColor.internalFormat = TexInterFormat::RGBA8;
		texSpecColor.dataFormat = TexDataFormat::RGBA;
		texSpecColor.dataType = TexDataType::UnsignedByte;
		texSpecColor.target = TexTarget::Texture2D;
		texSpecColor.wrapModeS = TexWrapMode::Clamp;
		texSpecColor.wrapModeT = TexWrapMode::Clamp;
		texSpecColor.filterModeMin = TexFilterMode::Nearest;
		texSpecColor.filterModeMag = TexFilterMode::Nearest;
		FBSpec fbSpec(1280u, 720u, { texSpecDepth, texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pickingPass->SetRenderTarget(fbo);
		pickingPass->SetRenderPassType(RenderPassType::Pick);
		return pickingPass;
	}

	RenderPass* RenderLayer::SetupSSRPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
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

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "SSR.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		TexSpec texSpecColor;
		{
			texSpecColor.internalFormat = TexInterFormat::RGBA8;
			texSpecColor.dataFormat = TexDataFormat::RGBA;
			texSpecColor.dataType = TexDataType::UnsignedByte;
			texSpecColor.target = TexTarget::Texture2D;
			texSpecColor.wrapModeS = TexWrapMode::Clamp;
			texSpecColor.wrapModeT = TexWrapMode::Clamp;
			texSpecColor.filterModeMin = TexFilterMode::Nearest;
			texSpecColor.filterModeMag = TexFilterMode::Nearest;
		}
		FBSpec fbSpec(1280u, 720u, { texSpecColor });
		auto TexO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(TexO);
		renderPass->SetRenderPassType(RenderPassType::SSRvs);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupHiZPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
			}
			shader->SetInt("u_Depth", 0);
			shader->SetInt("u_SelfDepth", 1);
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
		RenderPassDefault* HiZPass = new RenderPassDefault();
		HiZPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto depthShader = Shader::Create(currentPath / "ShaderSrc" / "HiZ.glsl");
		HiZPass->SetShader(depthShader);
		TexSpec depthAttachment;
		depthAttachment.target = TexTarget::Texture2D;
		depthAttachment.internalFormat = TexInterFormat::RED32F;
		depthAttachment.dataFormat = TexDataFormat::RED;
		depthAttachment.wrapModeS = TexWrapMode::Clamp;
		depthAttachment.wrapModeT = TexWrapMode::Clamp;
		depthAttachment.filterModeMin = TexFilterMode::NearestMipmapNearest;
		depthAttachment.filterModeMag = TexFilterMode::NearestMipmapNearest;
		depthAttachment.mipLevels = Utils::CalculateMaximumMipmapLevels(1280);
		FBSpec fbSpec(1280u, 720u, { depthAttachment });
		auto TexO = Framebuffer::Create(fbSpec);
		HiZPass->SetRenderTarget(TexO);
		HiZPass->SetRenderPassType(RenderPassType::HiZ);
		return HiZPass;
	}

	// Draw object outlines, lights, skeletons, etc.
	RenderPass* RenderLayer::SetupPostProcessingPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
			shader->Bind();
			renderTarget->EnableAttachments(0);
			RenderCommand::Clear(ClearFlags::Color_Buffer);
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
			}
			shader->SetUint("u_SelectedEntityID", GlobalState::g_SelectedEntityID);
			shader->SetInt("u_DrawDebug", GlobalState::g_ShowDebug);
			shader->SetInt("u_Result", 0);
			shader->SetInt("u_gEntity", 1);
			shader->SetInt("u_Debug", 2);
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		RenderPassDefault* postProcessingPass = new RenderPassDefault();
		postProcessingPass->SetRenderCommand(cmdBuffer);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto pp = Shader::Create(currentPath / "ShaderSrc" / "Postprocessing.glsl");
		postProcessingPass->SetShader(pp);
		TexSpec texSpecColor;
		texSpecColor.internalFormat = TexInterFormat::RGBA8;
		texSpecColor.dataFormat = TexDataFormat::RGBA;
		texSpecColor.dataType = TexDataType::UnsignedByte;
		texSpecColor.target = TexTarget::Texture2D;
		texSpecColor.wrapModeS = TexWrapMode::Clamp;
		texSpecColor.wrapModeT = TexWrapMode::Clamp;
		texSpecColor.filterModeMin = TexFilterMode::Nearest;
		texSpecColor.filterModeMag = TexFilterMode::Nearest;
		FBSpec fbSpec(1280u, 720u, { texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		postProcessingPass->SetRenderTarget(fbo);
		postProcessingPass->SetRenderPassType(RenderPassType::PostProcessing);
		return postProcessingPass;
	}

	RenderPass* RenderLayer::SetupShadowMapPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
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
				}
				else {
					shader->SetBool("u_IsInstanced", false);
					RenderCommand::DrawIndexed(data->GetVAO());
				}
				data->Unbind();
			}
			renderTarget->Unbind();
			shader->Unbind();
			});
		RenderPassDefault* depthRenderPass = new RenderPassDefault();
		depthRenderPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto depthShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMap.glsl");
		depthRenderPass->SetShader(depthShader);
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
		depthRenderPass->SetRenderTarget(depthTexO);
		depthRenderPass->SetRenderPassType(RenderPassType::Depth);
		return depthRenderPass;
	}
}