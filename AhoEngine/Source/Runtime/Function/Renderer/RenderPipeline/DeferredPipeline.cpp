#include "Ahopch.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "DeferredPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureUsage.h"
#include "Runtime/Function/Renderer/Texture/TextureConfig.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"


namespace Aho {
	void DeferredShading::Initialize() {
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc";
		
		// ---- Shadow Map ----
		{
			RenderPassConfig cfg;
			cfg.passName = "Shadow Map";
			cfg.shaderPath = (shaderPathRoot / "ShadowMap.glsl").string();

			std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(TextureConfig::GetDepthTextureConfig());
			m_TextureBuffers.push_back(depth);
			cfg.textureAttachments.push_back(depth.get());
			cfg.func =
				[&](RenderPassBase& self) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Stencil_Buffer);
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<VertexArrayComponent, _MaterialComponent, _TransformComponent>();
					auto shader = self.GetShader();
					auto fbo = self.GetRenderTarget();
					fbo->Bind();
					shader->Bind();
					view.each(
						[&shader](const auto& entity, const VertexArrayComponent& vao, const _MaterialComponent& mat, const _TransformComponent& transform) {
							vao.vao->Bind();
							shader->SetMat4("u_Model", transform.GetTransform());
							uint32_t bp = 0;
							RenderCommand::DrawIndexed(vao.vao);
							vao.vao->Unbind();
						}
					);
				};
			m_ShadowMapPass = std::make_unique<RenderPassBase>();
			m_ShadowMapPass->Setup(cfg);
		}

		// --- Shading Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Shading Pass";
			cfg.shaderPath = (shaderPathRoot / "Shading.glsl").string();

			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("Shading Result"));
			m_TextureBuffers.push_back(res);
			cfg.textureAttachments.push_back(res.get());

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					self.GetShader()->Bind();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);
					RenderCommand::DrawArray();
				};
			m_ShadingPass = std::make_unique<RenderPassBase>();
			m_ShadingPass->Setup(cfg);
			m_ResultTextureID = res->GetTextureID();
		}

		// --- G buffer Pass --
		{
			RenderPassConfig cfg;
			cfg.passName = "G-Buffer Pass";
			cfg.shaderPath = (shaderPathRoot / "GBuffer.glsl").string();

			std::shared_ptr<_Texture> position = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_Position"));
			m_TextureBuffers.push_back(position);
			cfg.textureAttachments.push_back(position.get());

			std::shared_ptr<_Texture> normal = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_Normal"));
			m_TextureBuffers.push_back(normal);
			cfg.textureAttachments.push_back(normal.get());

			std::shared_ptr<_Texture> baseColor = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_BaseColor"));
			m_TextureBuffers.push_back(baseColor);
			cfg.textureAttachments.push_back(baseColor.get());

			std::shared_ptr<_Texture> pbr = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_PBR"));
			m_TextureBuffers.push_back(pbr);
			cfg.textureAttachments.push_back(pbr.get());

			std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(TextureConfig::GetDepthTextureConfig("Depth"));
			m_TextureBuffers.push_back(depth);
			cfg.textureAttachments.push_back(depth.get());

			// --- Register G-Buffers to shading pass ---
			m_ShadingPass->RegisterTextureBuffer(position->GetTextureID(), "u_gPosition");
			m_ShadingPass->RegisterTextureBuffer(normal->GetTextureID(), "u_gNormal");
			m_ShadingPass->RegisterTextureBuffer(baseColor->GetTextureID(), "u_gAlbedo");
			m_ShadingPass->RegisterTextureBuffer(pbr->GetTextureID(), "u_gPBR");
			m_ShadingPass->RegisterTextureBuffer(depth->GetTextureID(), "u_gDepth");

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<VertexArrayComponent, _MaterialComponent, _TransformComponent>();
					auto shader = self.GetShader();
					auto fbo = self.GetRenderTarget();

					fbo->Bind();
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Stencil_Buffer | ClearFlags::Color_Buffer);
					shader->Bind();
					view.each(
						[&shader](const auto& entity, const VertexArrayComponent& vao, const _MaterialComponent& mat, const _TransformComponent& transform) {
							vao.vao->Bind();
							uint32_t bp = 0;
							_Material::ApplyToShader(shader, mat.mat, bp);
							shader->SetMat4("u_Model", transform.GetTransform());
							RenderCommand::DrawIndexed(vao.vao);
							vao.vao->Unbind();
						}
					);
				};
			m_GBufferPass = std::make_unique<RenderPassBase>();
			m_GBufferPass->Setup(cfg);
		}

	}

	void DeferredShading::Execute() {
		m_ShadowMapPass->Execute();
		m_GBufferPass->Execute();
		m_ShadingPass->Execute();
	}

}