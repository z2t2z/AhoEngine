#include "Ahopch.h"
#include "DeferredPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Core/Events/MainThreadDispatcher.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
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
			cfg.usage = ShaderUsage::DistantLightShadowMap;

			std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(TextureConfig::GetDepthTextureConfig());
			m_TextureBuffers.push_back(depth);
			cfg.textureAttachments.push_back(depth.get());
			cfg.func =
				[&](RenderPassBase& self) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Stencil_Buffer);
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<VertexArrayComponent, _MaterialComponent, _TransformComponent>();
					auto shader = self.GetShader();
					self.GetRenderTarget()->Bind();
					shader->Bind();
					view.each(
						[&shader](const auto& entity, const VertexArrayComponent& vao, const _MaterialComponent& mat, const _TransformComponent& transform) {
							vao.vao->Bind();
							shader->SetMat4("u_Model", transform.GetTransform());
							RenderCommand::DrawIndexed(vao.vao);
							vao.vao->Unbind();
						}
					);
					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};
			m_ShadowMapPass = std::make_unique<RenderPassBase>();
			m_ShadowMapPass->Setup(cfg);
		}

		// --- Shading Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Shading Pass";
			cfg.shaderPath = (shaderPathRoot / "Shading.glsl").string();
			cfg.usage = ShaderUsage::DeferredShading;

			auto texCfg = TextureConfig::GetColorTextureConfig("Shading Result");
			texCfg.InternalFmt = InternalFormat::RGBA16F; // Use HDR format for shading result
			texCfg.DataFmt = DataFormat::RGBA;
			texCfg.DataType = DataType::Float;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			m_TextureBuffers.push_back(res);
			cfg.textureAttachments.push_back(res.get());

			cfg.func =
				[](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto shader = self.GetShader();
					shader->Bind();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);
					auto view = ecs->GetView<AtmosphereParametersComponent, LightComponent>();
					view.each(
						[&shader, &slot](auto _, const AtmosphereParametersComponent& atmosphere, const LightComponent& lightCmp) {
							const std::shared_ptr<DirectionalLight>& light = std::static_pointer_cast<DirectionalLight>(lightCmp.light);
							shader->SetVec3("u_SunDir", light->GetDirection());
							shader->SetInt("u_SkyviewLUT", slot);
							RenderCommand::BindTextureUnit(slot++, atmosphere.SkyViewTextureID);
						}
					);
					auto iblView = ecs->GetView<IBLComponent>();
					shader->SetBool("u_SampleEnvLight", !iblView.empty());
					iblView.each(
						[&](auto _, const IBLComponent& iblComp) {
							if (iblComp.EnvTextureSkyBox) {
								shader->SetInt("u_gCubeMap", slot);
								RenderCommand::BindTextureUnit(slot++, iblComp.EnvTextureSkyBox->GetTextureID());
								shader->SetInt("u_gIrradiance", slot);
								RenderCommand::BindTextureUnit(slot++, iblComp.Irradiance->GetTextureID());
								shader->SetInt("u_gPrefilter", slot);
								RenderCommand::BindTextureUnit(slot++, iblComp.Prefilter->GetTextureID());
								shader->SetInt("u_gBRDFLUT", slot);
								RenderCommand::BindTextureUnit(slot++, iblComp.BRDFLUT->GetTextureID());
								shader->SetFloat("u_PrefilterMaxMipLevel", iblComp.Prefilter->GetMipLevels());
							}
						}
					);
					RenderCommand::DrawArray();
					shader->Unbind();
					self.GetRenderTarget()->Unbind();
				};
			m_ShadingPass = std::make_unique<RenderPassBase>();
			m_ShadingPass->Setup(cfg);

			auto renderer = g_RuntimeGlobalCtx.m_Renderer;
			auto skyPipeline = g_RuntimeGlobalCtx.m_Renderer->GetSkyAtmosphericPipeline();
			m_ShadingPass->RegisterTextureBuffer(skyPipeline->GetTextureBufferByIndex(2), "u_SkyviewLUT");
			
			m_ResultTextureID = res->GetTextureID();
			m_Result = res.get();
		}

		// --- G buffer Pass --
		{
			RenderPassConfig cfg;
			cfg.passName = "G-Buffer Pass";
			cfg.shaderPath = (shaderPathRoot / "GBuffer.glsl").string();
			cfg.usage = ShaderUsage::GBuffer;

			auto posTexCfg = TextureConfig::GetColorTextureConfig("G_Position");
			posTexCfg.InternalFmt = InternalFormat::RGB16F; // Use HDR format for shading result
			posTexCfg.DataFmt = DataFormat::RGB;
			posTexCfg.DataType = DataType::Float;
			std::shared_ptr<_Texture> position = std::make_shared<_Texture>(posTexCfg);
			m_TextureBuffers.push_back(position);
			cfg.textureAttachments.push_back(position.get());

			auto normalTexCfg = TextureConfig::GetColorTextureConfig("G_Normal");
			normalTexCfg.InternalFmt = InternalFormat::RGB16F; // Use HDR format for shading result
			normalTexCfg.DataFmt = DataFormat::RGB;
			normalTexCfg.DataType = DataType::Float;
			std::shared_ptr<_Texture> normal = std::make_shared<_Texture>(normalTexCfg);
			m_TextureBuffers.push_back(normal);
			cfg.textureAttachments.push_back(normal.get());
			
			std::shared_ptr<_Texture> baseColor = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_BaseColor"));
			m_TextureBuffers.push_back(baseColor);
			cfg.textureAttachments.push_back(baseColor.get());

			auto pbrTex = TextureConfig::GetColorTextureConfig("G_PBR");
			pbrTex.InternalFmt = InternalFormat::RGB16F; 
			pbrTex.DataFmt = DataFormat::RGB;
			pbrTex.DataType = DataType::Float;
			std::shared_ptr<_Texture> pbr = std::make_shared<_Texture>(pbrTex);
			m_TextureBuffers.push_back(pbr);
			cfg.textureAttachments.push_back(pbr.get());

			std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(TextureConfig::GetDepthTextureConfig("Depth"));
			m_TextureBuffers.push_back(depth);
			cfg.textureAttachments.push_back(depth.get());

			// --- Register G-Buffers to shading pass ---
			m_ShadingPass->RegisterTextureBuffer(position.get(), "u_gPosition");
			m_ShadingPass->RegisterTextureBuffer(normal.get(), "u_gNormal");
			m_ShadingPass->RegisterTextureBuffer(baseColor.get(), "u_gAlbedo");
			m_ShadingPass->RegisterTextureBuffer(pbr.get(), "u_gPBR");
			m_ShadingPass->RegisterTextureBuffer(depth.get(), "u_gDepth");

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
							uint32_t slot = 0;
							mat.mat.ApplyToShader(shader, slot);
							shader->SetMat4("u_Model", transform.GetTransform());
							RenderCommand::DrawIndexed(vao.vao);
							vao.vao->Unbind();
						}
					);
					fbo->Unbind();
					shader->Unbind();
				};
			m_GBufferPass = std::make_unique<RenderPassBase>();
			m_GBufferPass->Setup(cfg);
		}

		return;
		// --- Screen Space Reflection Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Screen Space Reflection Pass";
			cfg.shaderPath = (shaderPathRoot / "SSR.glsl").string();
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
			m_SSRPass = std::make_unique<RenderPassBase>();
			m_SSRPass->Setup(cfg);
		}
	}

	void DeferredShading::Execute() {
		m_ShadowMapPass->Execute();
		m_GBufferPass->Execute();
		m_ShadingPass->Execute();
	}

	bool DeferredShading::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_GBufferPass->Resize(width, height);
		//m_SSAOPass;
		//m_BlurRPass;
		resized |= m_ShadingPass->Resize(width, height);
		return resized;
	}

}