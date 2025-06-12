#include "Ahopch.h"
#include "PostprocessPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Resource/FileWatcher/FileWatcher.h"
#include "Runtime/Function/Renderer/Shader/ShaderVariantManager.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"
#include "Runtime/Function/Renderer/Texture/TextureResourceBuilder.h"
#include "../../../../../../AhoEditor/Source/EditorUI/EditorGlobalContext.h"

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	void PostprocessPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_PostProcess;

		m_SelectedDepth = TextureResourceBuilder()
			.Name("SelectedDepth").Width(1280).Height(720).DataType(DataType::UShort).DataFormat(DataFormat::Depth).InternalFormat(InternalFormat::Depth16)
			.Build();

		m_Outlined = TextureResourceBuilder()
			.Name("Outlined").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();
		m_TextureBuffers.push_back(m_SelectedDepth);
		m_TextureBuffers.push_back(m_Outlined);


		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "Postprocessing";
		// Draw selected object into a depth buffer for editor outline
		{
			auto Func =
				[](RenderPassBase& self) {
					if (!g_EditorGlobalCtx.HasActiveSelected()) 
						return;
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Depth_Buffer);
					auto shader = self.GetShader();
					shader->Bind();
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					Entity entity = g_EditorGlobalCtx.GetSelectedEntity();

					auto [vertexArray, transform] = ecs->TryGet<VertexArrayComponent, _TransformComponent>(entity);
					if (vertexArray && transform) {
						vertexArray->vao->Bind();
						shader->SetMat4("u_Model", transform->GetTransform());
						//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
						RenderCommand::DrawIndexed(vertexArray->vao);
						vertexArray->vao->Unbind();
					}

					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};
			m_SingleDepthPass = std::move(RenderPassBuilder()
				.Name("SingleDepthPass")
				.Shader((shaderPathRoot / "SingleDepth.glsl").string())
				//.Usage(ShaderUsage::DistantLightShadowMap)
				.AttachTarget(m_SelectedDepth)
				.Func(Func)
				.Build());
		}

		// Outline pass
		{
			auto Func =
				[](RenderPassBase& self) {
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					auto shader = self.GetShader();
					self.GetRenderTarget()->Bind();
					shader->Bind();
					
					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);

					auto renderMode = g_RuntimeGlobalCtx.m_Renderer->GetRenderMode();
					shader->SetInt("u_RenderMode", renderMode == RenderMode::PathTracing ? 1 : 0);

					glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
					RenderCommand::DrawArray();

					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};
			m_OutlinePass = std::move(RenderPassBuilder()
				.Name("OutlinePass")
				.Shader((shaderPathRoot / "EditorOutline.glsl").string())
				.AttachTarget(m_Outlined)
				.Input("u_SelectedDepth", m_SelectedDepth)
				.Input("u_Scene", g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("Lit Scene Result"))
				.Input("u_PT_Scene", g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("PathTracingPresent"))
				.Func(Func)
				.Build());
		}

		m_Result = m_Outlined.get();
	}

	void PostprocessPipeline::Execute() {
		m_SingleDepthPass->Execute();
		m_OutlinePass->Execute();
	}

	bool PostprocessPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_SelectedDepth->Resize(width, height);
		resized |= m_Outlined->Resize(width, height);
		return resized;
	}

	// ----- delete these -----
	std::unique_ptr<RenderPass> PostprocessPipeline::SetupDrawSelectedOutlinePass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->Bind();
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				//shader->SetUint("u_SelectedEntityID", RendererGlobalState::g_SelectedEntityID);
				//shader->SetBool("u_IsEntityIDValid", RendererGlobalState::g_IsEntityIDValid);

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

	std::unique_ptr<RenderPass> PostprocessPipeline::SetupFXAAPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->Bind();
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
