#pragma once

#include "Ahopch.h"
#include "SkyAtmosphericPipeline.h"
#include "DeferredPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/Lights.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureResourceBuilder.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"

namespace Aho {
	SkyAtmosphericPipeline::SkyAtmosphericPipeline() {
		Initialize();
	}

	void SkyAtmosphericPipeline::Initialize() {
		// Buffers
		std::shared_ptr<_Texture> transmittanceLUT = TextureResourceBuilder()
			.Name("SkyTransmittanceLUT").Width(256).Height(64).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();

		std::shared_ptr<_Texture> multiScattLUT = TextureResourceBuilder()
			.Name("SkyMultiScattLUT").Width(32).Height(32).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();

		std::shared_ptr<_Texture> skyViewLUT = TextureResourceBuilder()
			.Name("SkViewLUT").Width(192).Height(108).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();

		m_TextureBuffers.push_back(transmittanceLUT);
		m_TextureBuffers.push_back(multiScattLUT);
		m_TextureBuffers.push_back(skyViewLUT);

		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "AtmosphericScattering";
		// -- Transmittiance lut pass --
		{
			auto Fnc =
				[](RenderPassBase& self) {
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

			m_TransmittanceLutPass = std::move(RenderPassBuilder()
				.Name("SkyTransmittanceLUT Pass")
				.Shader((shaderPathRoot / "Transmittance.glsl").string())
				//.Usage(ShaderUsage::Transtim)
				.AttachTarget(transmittanceLUT)
				.Func(Fnc)
				.Build());
		}

		// --- Mutiscattering pass ---
		{
			auto Func =
				[](RenderPassBase& self) {
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

			m_MutiScattLutPass = std::move(RenderPassBuilder()
				.Name("SkyMultiScattLUT Pass")
				.Shader((shaderPathRoot / "MutiScatt.glsl").string())
				//.Usage(ShaderUsage::DistantLightShadowMap)
				.AttachTarget(multiScattLUT)
				.Input("u_TransmittanceLUT", transmittanceLUT)
				.Func(Func)
				.Build());
		}

		// --- SkyView Pass ---
		{
			auto Func =
				[skyViewLUT](RenderPassBase& self) {
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

							atmosphere.SkyViewTextureID = skyViewLUT->GetTextureID();
						}
					);
				};

			m_SkyViewLutPass = std::move(RenderPassBuilder()
				.Name("SkyViewtLUT Pass")
				.Shader((shaderPathRoot / "SkyView.glsl").string())
				//.Usage(ShaderUsage::DistantLightShadowMap)
				.AttachTarget(skyViewLUT)
				.Input("u_TransmittanceLUT", transmittanceLUT)
				.Input("u_MultiScattLUT", multiScattLUT)
				.Func(Func)
				.Build());
		}
	}

	void SkyAtmosphericPipeline::Execute() {
		m_TransmittanceLutPass->Execute();
		m_MutiScattLutPass->Execute();
		m_SkyViewLutPass->Execute();
	}
}