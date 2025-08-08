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
		m_Grid = TextureResourceBuilder()
			.Name("SceneWithDbgGrid").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();
		m_ObjectID = TextureResourceBuilder()
			.Name("ObjectID").Width(1280).Height(720).DataType(DataType::UInt).DataFormat(DataFormat::RedInteger).InternalFormat(InternalFormat::R32UI)
			.Build();
		m_PickingDepth = TextureResourceBuilder()
			.Name("PickingPassDepth").Width(1280).Height(720).DataType(DataType::UShort).DataFormat(DataFormat::Depth).InternalFormat(InternalFormat::Depth16)
			.Build();

		m_TextureBuffers.push_back(m_SelectedDepth);
		m_TextureBuffers.push_back(m_Outlined);
		m_TextureBuffers.push_back(m_ObjectID);
		m_TextureBuffers.push_back(m_PickingDepth);


		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "Postprocessing";

		// --- Tone Mapping Pass --
		{

		}

		// Picking Pass, draw object id into a buffer for reading
		{
			auto Func =
				[](RenderPassBase& self) {
					auto [shouldPick, x, y] = g_EditorGlobalCtx.PickInfoAtThisFrame();
					if (!shouldPick)
						return;

					auto shader = self.GetShader();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
					shader->Bind();
					g_RuntimeGlobalCtx.m_Renderer->GetRenderableContext().each(
						[&shader](const Entity& entity, const VertexArrayComponent& vao, const _MaterialComponent& mat, const _TransformComponent& transform) {
							vao.vao->Bind();
							shader->SetUint("u_ObjectID", static_cast<uint32_t>(entity.GetEntityHandle()));
							shader->SetMat4("u_Model", transform.GetTransform());
							RenderCommand::DrawIndexed(vao.vao);
							vao.vao->Unbind();
						}
					);

					uint32_t pickedID;
					glReadBuffer(GL_COLOR_ATTACHMENT0);
					glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pickedID);
					g_EditorGlobalCtx.SetSelected(pickedID);

					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};

			m_PickingPass = std::move(RenderPassBuilder()
				.Name("PickingPass")
				.Shader((shaderPathRoot / "DrawObjectID.glsl").string())
				//.Usage(ShaderUsage::DistantLightShadowMap)
				.AttachTarget(m_ObjectID)
				.AttachTarget(m_PickingDepth)
				.Func(Func)
				.Build());
		}

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
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					auto shader = self.GetShader();
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
				//.Input("u_PT_Scene", g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("PathTracingPresent"))
				.Input("u_PT_Scene", g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("WaveFrontPathTracingPresent"))
				.Func(Func)
				.Build());
		}

		// Draw an infinite grid in the scene for good looking
		{
			auto Func =
				[](RenderPassBase& self) {
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					
					auto shader = self.GetShader();
					shader->Bind();

					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);
					glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
					RenderCommand::DrawArray();

					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};
			m_DrawGridPass = std::move(RenderPassBuilder()
				.Name("DrawInfiniteGrid"))
				.Shader((shaderPathRoot / "InfiniteGrid.glsl").string())
				.AttachTarget(m_Grid)
				.Input("u_Scene", m_Outlined)
				.Input("u_SceneDepth", g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("Scene Depth"))
				.Func(Func)
				.Build();
		}

		m_Result = m_Grid.get();
	}

	void PostprocessPipeline::Execute() {
		m_PickingPass->Execute();
		m_SingleDepthPass->Execute();
		m_OutlinePass->Execute();
		m_DrawGridPass->Execute();
	}

	bool PostprocessPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_PickingPass->Resize(width, height);
		resized |= m_SelectedDepth->Resize(width, height);
		resized |= m_Outlined->Resize(width, height);
		resized |= m_Grid->Resize(width, height);
		return resized;
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
