#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
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
		for (const auto& pass : *(m_Renderer->GetCurrentRenderPipeline())) {
			pass->BindSceneDataUBO(m_UBO);
		}
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
		//pipeline->AddRenderPass(SetupSSAOGeoPass());
		m_Renderer->SetCurrentRenderPipeline(pipeline);
	}

	RenderPass* RenderLayer::SetupSSAOGeoPass() {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
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
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
			uint32_t texOffset = 0u;
			for (const auto& texBuffer : textureBuffers) {
				texBuffer->Bind(texOffset++);
			}
			//shader->SetInt("u_")
			for (const auto& data : renderData) {
				data->Bind(shader, texOffset);
				RenderCommand::DrawIndexed(data->GetVAO());
				data->Unbind();
			}
		});

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		auto shader = Shader::Create((currentPath / "ShaderSrc" / "SSAO.glsl").string());
		RenderPassDefault* renderPass = new RenderPassDefault();
		AHO_CORE_ASSERT(shader->IsCompiled());
		renderPass->SetShader(shader);
		renderPass->SetRenderCommand(cmdBuffer);
		FBTextureSpecification ssaoColor;
		FBTextureSpecification texSpecDepth; texSpecDepth.dataFormat = FBDataFormat::DepthComponent;
		ssaoColor.internalFormat = FBInterFormat::RED;
		ssaoColor.dataFormat = FBDataFormat::RED;
		ssaoColor.dataType = FBDataType::UnsignedByte;
		ssaoColor.target = FBTarget::Texture2D;
		ssaoColor.filterModeMin = FBFilterMode::Nearest;
		ssaoColor.filterModeMag = FBFilterMode::Nearest;
		FBTextureSpecification ssaoBlur = ssaoColor;
		FBSpecification fbSpec(1280u, 720u, { ssaoColor, ssaoBlur });
		auto FBO = Framebuffer::Create(fbSpec);
		renderPass->SetRenderTarget(FBO);
		renderPass->SetRenderPassType(RenderPassType::Final);
		return renderPass;
	}

	RenderPass* RenderLayer::SetupDebugPass(const std::shared_ptr<Framebuffer>& fbo) {
		RenderCommandBuffer* cmdBuffer = new RenderCommandBuffer();
		cmdBuffer->SetClearColor(glm::vec4(0.1f, 0.15f, 0.15f, 1.0f));
		RenderPassDefault* debugPass = new RenderPassDefault();
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
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
		cmdBuffer->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
			uint32_t texOffset = 0u;
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

		std::filesystem::path currentPath = std::filesystem::current_path(); // TODO: Shoule be inside a config? or inside a global settings struct
		std::string FileName = (currentPath / "ShaderSrc" / "pbrShader.glsl").string();
		auto shader = Shader::Create(FileName);
		RenderPassDefault* renderPass = new RenderPassDefault();
		if (shader->IsCompiled()) {
			renderPass->SetShader(shader);
		}
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
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
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
		cmdBufferDepth->AddCommand([](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<Texture*>& textureBuffers) {
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