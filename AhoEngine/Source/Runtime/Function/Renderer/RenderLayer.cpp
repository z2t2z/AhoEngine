#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"
#include <imgui.h>

namespace Aho {
	RenderLayer::RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("RenderLayer"), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void RenderLayer::OnAttach() {
		AHO_CORE_INFO("Renderer on attach");
		SetupForwardRenderPipeline();
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
					pipeLine->AddVirtualRenderData(data);
					if (!data->IsVirtual()) {
						pipeLine->AddRenderData(data); // TODO: Memory wastage, should be optimized
					}
				}
			}
		}
	}

	void RenderLayer::SetupForwardRenderPipeline() {
		RenderPipelineDefault* pipeline = new RenderPipelineDefault();
		auto mainPass = SetupMainPass();
		pipeline->AddRenderPass(mainPass);
		auto depthPass = SetupDepthPass();
		auto depthMapFBO = depthPass->GetRenderTarget();
		pipeline->AddRenderPass(depthPass);
		Texture* depthTex = depthPass->GetRenderTarget()->GetDepthTexture();
		mainPass->AddGBuffer(depthTex);
		auto debugPass = SetupDebugPass(depthMapFBO);
		debugPass->AddGBuffer(depthTex);
		pipeline->AddRenderPass(debugPass);
		pipeline->AddRenderPass(SetupPickingPass());

		// Setup SSAO
		auto ssaoGeoPass = SetupSSAOGeoPass();
		auto attachments = ssaoGeoPass->GetRenderTarget()->GetTextureAttachments();
		auto ssaoPass = SetupSSAOPass();
		auto ssaoLighting = SetupSSAOLightingPass();
		for (int i = 0; i < 3; i++) {
			// Note that order matters: gPostion, gNormal, gAlbedo, gSSAO
			if (i < 2) {
				ssaoPass->AddGBuffer(attachments[i]);
			}	
			//ssaoLighting->AddGBuffer(attachments[i]);
		}
		// ssaoPass: gPostion, gNormal, gNoise
		ssaoPass->AddGBuffer(Utils::CreateNoiseTexture(32));
		// ssaoPass: gPostion, gNormal, gAlbedo, gSSAO
		ssaoLighting->AddGBuffer(ssaoPass->GetRenderTarget()->GetTextureAttachments().back());
		auto blurPass = SetupSSAOBlurPass();
		blurPass->AddGBuffer(ssaoPass->GetRenderTarget()->GetTextureAttachments().back());
		mainPass->AddGBuffer(blurPass->GetRenderTarget()->GetTextureAttachments().back());
		pipeline->AddRenderPass(blurPass);
		pipeline->AddRenderPass(ssaoGeoPass);
		pipeline->AddRenderPass(ssaoPass);
		pipeline->AddRenderPass(ssaoLighting);
		// Setup UBO data, order matters!!
		OpenGLShader::SetUBO(sizeof(UBO), 0, DrawType::Dynamic);
		OpenGLShader::SetUBO(sizeof(GeneralUBO), 1, DrawType::Dynamic);
		OpenGLShader::SetUBO(sizeof(SSAOUBO), 2, DrawType::Dynamic);
		pipeline->AddUBO((void*)new UBO());
		pipeline->AddUBO((void*)new GeneralUBO());
		pipeline->AddUBO((void*)new SSAOUBO());
		m_Renderer->SetCurrentRenderPipeline(pipeline);
	}

	RenderPass* RenderLayer::SetupSSAOGeoPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 0, sizeof(UBO));
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		auto shader = Shader::Create((currentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		RenderPassDefault* renderPass = new RenderPassDefault();
		AHO_CORE_ASSERT(shader->IsCompiled());
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification positionAttachment;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		positionAttachment.internalFormat = FBInterFormat::RGBA16F;
		positionAttachment.dataFormat = FBDataFormat::RGBA;
		positionAttachment.dataType = FBDataType::Float;
		positionAttachment.target = FBTarget::Texture2D;
		positionAttachment.wrapModeS = FBWrapMode::Clamp;
		positionAttachment.wrapModeT = FBWrapMode::Clamp;
		positionAttachment.filterModeMin = FBFilterMode::Nearest;
		positionAttachment.filterModeMag = FBFilterMode::Nearest;
		FBTextureSpecification normalAttachment = positionAttachment;
		normalAttachment.wrapModeS = FBWrapMode::None;
		FBTextureSpecification colorAttachment = normalAttachment;
		colorAttachment.dataType = FBDataType::UnsignedByte;
		colorAttachment.internalFormat = FBInterFormat::RGBA8;
		FBSpecification fbSpec(1280u, 720u, { positionAttachment, normalAttachment, colorAttachment, texSpecDepth });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::SSAOGeo);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		//cmdBuffer->SetClearFlags(ClearFlags::Color_Buffer);
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 2, sizeof(SSAOUBO));
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
		});
		cmdBuffer->SetClearFlags(ClearFlags::Color_Buffer);
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		auto shader = Shader::Create((currentPath / "ShaderSrc" / "SSAO.glsl").string());
		RenderPassDefault* renderPass = new RenderPassDefault();
		AHO_CORE_ASSERT(shader->IsCompiled());
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification ssaoColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		{
			ssaoColor.internalFormat = FBInterFormat::RED;
			ssaoColor.dataFormat = FBDataFormat::RED;
			ssaoColor.dataType = FBDataType::Float;
			ssaoColor.target = FBTarget::Texture2D;
			ssaoColor.filterModeMin = FBFilterMode::Nearest;
			ssaoColor.filterModeMag = FBFilterMode::Nearest;
		}
		FBSpecification fbSpec(1280u, 720u, {ssaoColor});
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::SSAO);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOBlurPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
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
		});
		cmdBuffer->SetClearFlags(ClearFlags::Color_Buffer);
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "Blur.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification texSpecColor;
		{
			texSpecColor.internalFormat = FBInterFormat::RED;
			texSpecColor.dataFormat = FBDataFormat::RED;
			texSpecColor.dataType = FBDataType::Float;
			texSpecColor.target = FBTarget::Texture2D;
			texSpecColor.wrapModeS = FBWrapMode::Clamp;
			texSpecColor.wrapModeT = FBWrapMode::Clamp;
			texSpecColor.filterModeMin = FBFilterMode::Nearest;
			texSpecColor.filterModeMag = FBFilterMode::Nearest;
		}
		FBSpecification fbSpec(1280u, 720u, { texSpecColor });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::Blur);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupSSAOLightingPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 1, sizeof(GeneralUBO));
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // Note that order matters!
			}
			shader->SetInt("u_gPosition", 0);
			shader->SetInt("u_gNormal", 1);
			shader->SetInt("u_gAlbedo", 2);
			shader->SetInt("u_gSSAO", 3);
			//shader->SetInt("u_DepthMap", texOffset++);
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
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		{
			texSpecColor.internalFormat = FBInterFormat::RGBA8;
			texSpecColor.dataFormat = FBDataFormat::RGBA;
			texSpecColor.dataType = FBDataType::UnsignedByte;
			texSpecColor.target = FBTarget::Texture2D;
			texSpecColor.wrapModeS = FBWrapMode::Clamp;
			texSpecColor.wrapModeT = FBWrapMode::Clamp;
			texSpecColor.filterModeMin = FBFilterMode::Nearest;
			texSpecColor.filterModeMag = FBFilterMode::Nearest;
		}
		FBSpecification fbSpec(1280u, 720u, { texSpecColor });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::SSAOLighting);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupDebugPass(const std::shared_ptr<Framebuffer>& fbo) {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->SetClearColor(glm::vec4(0.1f, 0.15f, 0.15f, 1.0f));
		RenderPassDefault* debugPass = new RenderPassDefault();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			uint32_t texOffset = 0u;
			AHO_CORE_ASSERT(textureBuffers.size() == 1);
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset);
			}
			shader->SetInt("u_DepthMap", texOffset++);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		debugPass->SetRenderCommand(cmdBuffer);
		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		const auto debugShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMapDebug.glsl");
		debugPass->SetShader(debugShader);
		debugPass->SetRenderPassType(RenderPassType::Debug);
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecColor.internalFormat = FBInterFormat::RGBA8;
		texSpecColor.dataFormat = FBDataFormat::RGBA;
		texSpecColor.dataType = FBDataType::UnsignedByte;
		texSpecColor.target = FBTarget::Texture2D;
		texSpecColor.wrapModeS = FBWrapMode::Clamp;
		texSpecColor.wrapModeT = FBWrapMode::Clamp;
		texSpecColor.filterModeMin = FBFilterMode::Nearest;
		texSpecColor.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1280u, 720u, { texSpecColor });
		auto FBO = Framebuffer::Create(fbSpec);
		debugPass->SetRenderTarget(FBO);
		return debugPass;
	}
	
	RenderPass* RenderLayer::SetupMainPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 1, sizeof(GeneralUBO));
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++); // TODO;
			}
			shader->SetInt("u_DepthMap", 0);
			shader->SetInt("u_SSAO", 1);
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "pbrShader.glsl").string();
		auto shader = Shader::Create(FileName);
		AHO_CORE_ASSERT(shader->IsCompiled());
		RenderPassDefault* renderPass = new RenderPassDefault();
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecColor.internalFormat = FBInterFormat::RGBA8;
		texSpecColor.dataFormat = FBDataFormat::RGBA;
		texSpecColor.dataType = FBDataType::UnsignedByte;
		texSpecColor.target = FBTarget::Texture2D;
		texSpecColor.wrapModeS = FBWrapMode::Clamp;
		texSpecColor.wrapModeT = FBWrapMode::Clamp;
		texSpecColor.filterModeMin = FBFilterMode::Nearest;
		texSpecColor.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1280u, 720u, { texSpecColor, texSpecDepth });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::Final);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupPickingPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 0, sizeof(UBO));
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		RenderPassDefault* pickingPass = new RenderPassDefault();
		pickingPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto pickingShader = Shader::Create(currentPath / "ShaderSrc" / "MousePicking.glsl");
		pickingPass->SetShader(pickingShader);
		FBTextureSpecification texSpecColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecColor.internalFormat = FBInterFormat::RGBA8;
		texSpecColor.dataFormat = FBDataFormat::RGBA;
		texSpecColor.dataType = FBDataType::UnsignedByte;
		texSpecColor.target = FBTarget::Texture2D;
		texSpecColor.wrapModeS = FBWrapMode::Clamp;
		texSpecColor.wrapModeT = FBWrapMode::Clamp;
		texSpecColor.filterModeMin = FBFilterMode::Nearest;
		texSpecColor.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(1280u, 720u, { texSpecDepth, texSpecColor });  // pick pass can use a low resolution. But ratio should be the same
		auto fbo = Framebuffer::Create(fbSpec);
		pickingPass->SetRenderTarget(fbo);
		pickingPass->SetRenderPassType(RenderPassType::Pick);
		return pickingPass;
	}

	RenderPass* RenderLayer::SetupDepthPass() {
		RenderCommandBuffer* cmdBufferDepth = new RenderCommandBuffer();
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers, const void* ubo) {
			shader->BindUBO(ubo, 0, sizeof(UBO));
			for (const auto& data : renderData) {
				data->Bind(shader);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});
		RenderPassDefault* depthRenderPass = new RenderPassDefault();
		depthRenderPass->SetRenderCommand(cmdBufferDepth);
		std::filesystem::path currentPath = std::filesystem::current_path();
		auto depthShader = Shader::Create(currentPath / "ShaderSrc" / "ShadowMap.glsl");
		depthRenderPass->SetShader(depthShader);
		FBTextureSpecification texSpecDepth;
		texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		texSpecDepth.internalFormat = FBInterFormat::Depth24;	// ......
		texSpecDepth.dataType = FBDataType::Float;
		texSpecDepth.wrapModeS = FBWrapMode::Repeat;
		texSpecDepth.wrapModeT = FBWrapMode::Repeat;
		texSpecDepth.filterModeMin = FBFilterMode::Nearest;
		texSpecDepth.filterModeMag = FBFilterMode::Nearest;
		FBSpecification fbSpec(2048, 2048, { texSpecDepth });  // Hardcode fow now
		auto depthFBO = Framebuffer::Create(fbSpec);
		depthRenderPass->SetRenderTarget(depthFBO);
		depthRenderPass->SetRenderPassType(RenderPassType::Depth);
		return depthRenderPass;
	}
}