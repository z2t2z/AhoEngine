#pragma once

#include "Ahopch.h"
#include "SkyAtmosphericPipeline.h"
#include "DeferredPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/Lights.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureUsage.h"
#include "Runtime/Function/Renderer/Texture/TextureConfig.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"


namespace Aho {
	SkyAtmosphericPipeline::SkyAtmosphericPipeline() {
		Initialize();
	}

	void SkyAtmosphericPipeline::Initialize() {
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "AtmosphericScattering";

		// -- Transmittiance lut pass --
		{
			RenderPassConfig cfg;
			cfg.passName = "Sky Transmittance LUT Pass";
			cfg.shaderPath = (shaderPathRoot / "Transmittance.glsl").string();

			TextureConfig texCfg = TextureConfig::GetColorTextureConfig("SkyTransmittanceLUT");
			texCfg.InternalFmt = InternalFormat::RGBA16F;
			texCfg.DataType = DataType::Float;
			texCfg.Width = 256; texCfg.Height = 64;

			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			m_TextureBuffers.push_back(res);
			cfg.attachmentTargets.push_back(res.get());

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<AtmosphereParametersComponent, LightComponent>();
					view.each(
						[&](auto entity, const AtmosphereParametersComponent& atmosphere, const LightComponent& lightCmp) {
							auto shader = self.GetShader();
							shader->Bind();
							self.GetRenderTarget()->Bind();

							RenderCommand::Clear(ClearFlags::Color_Buffer);
							
							glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
							uint32_t slot = 0;
							self.BindRegisteredTextureBuffers(slot);
							
							RenderCommand::DrawArray();

							shader->Unbind();
							self.GetRenderTarget()->Unbind();
						}
					);
				};
			m_TransmittanceLutPass = std::make_unique<RenderPassBase>();
			m_TransmittanceLutPass->Setup(cfg);
		}

		// --- Mutiscattering pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Sky MultiScattLUT Pass";
			cfg.shaderPath = (shaderPathRoot / "MutiScatt.glsl").string();
			
			TextureConfig texCfg = TextureConfig::GetColorTextureConfig("SkyMultiScattLUT");
			texCfg.InternalFmt = InternalFormat::RGBA16F;
			texCfg.Width = 32; texCfg.Height = 32;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);

			m_TextureBuffers.push_back(res);
			cfg.attachmentTargets.push_back(res.get());
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<AtmosphereParametersComponent, LightComponent>();
					view.each(
						[&](auto entity, const AtmosphereParametersComponent& atmosphere, const LightComponent& lightCmp) {
							auto shader = self.GetShader();
							shader->Bind();

							self.GetRenderTarget()->Bind();
							RenderCommand::Clear(ClearFlags::Color_Buffer);
							
							glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
							uint32_t slot = 0;
							self.BindRegisteredTextureBuffers(slot);
							
							RenderCommand::DrawArray();

							shader->Unbind();
							self.GetRenderTarget()->Unbind();
						}
					);
				};
			m_MutiScattLutPass = std::make_unique<RenderPassBase>();
			m_MutiScattLutPass->Setup(cfg);
			m_MutiScattLutPass->RegisterTextureBuffer(m_TextureBuffers[0].get(), "u_TransmittanceLUT");
		}

		// --- SkyView Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Sky View LUT Pass";
			cfg.shaderPath = (shaderPathRoot / "SkyView.glsl").string();

			TextureConfig texCfg = TextureConfig::GetColorTextureConfig("SkyViewLUT");
			texCfg.InternalFmt = InternalFormat::RGBA16F;
			texCfg.Width = 192; texCfg.Height = 108;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			
			m_TextureBuffers.push_back(res);
			cfg.attachmentTargets.push_back(res.get());
			cfg.func =
				[res](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<AtmosphereParametersComponent, LightComponent>();
					view.each(
						[&](auto entity, AtmosphereParametersComponent& atmosphere, const LightComponent& lightCmp) {
							auto shader = self.GetShader();
							shader->Bind();

							const std::shared_ptr<DirectionalLight>& light = std::static_pointer_cast<DirectionalLight>(lightCmp.light);
							shader->SetVec3("u_SunDir", light->GetDirection());

							self.GetRenderTarget()->Bind();
							RenderCommand::Clear(ClearFlags::Color_Buffer);

							glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
							uint32_t slot = 0;
							self.BindRegisteredTextureBuffers(slot);

							RenderCommand::DrawArray();

							shader->Unbind();
							self.GetRenderTarget()->Unbind();

							atmosphere.SkyViewTextureID = res->GetTextureID();
						}
					);
				};
			m_SkyViewLutPass = std::make_unique<RenderPassBase>();
			m_SkyViewLutPass->Setup(cfg);
			m_SkyViewLutPass->RegisterTextureBuffer(m_TextureBuffers[0].get(), "u_TransmittanceLUT");
			m_SkyViewLutPass->RegisterTextureBuffer(m_TextureBuffers[1].get(), "u_MultiScattLUT");
		}
	}

	void SkyAtmosphericPipeline::Execute() {
		m_TransmittanceLutPass->Execute();
		m_MutiScattLutPass->Execute();
		m_SkyViewLutPass->Execute();
	}
}