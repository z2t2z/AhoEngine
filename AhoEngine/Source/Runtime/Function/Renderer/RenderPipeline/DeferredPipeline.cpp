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

// Test
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"

namespace Aho {
	void DeferredShading::Initialize() {
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc";
		
		// ---- Shadow Map ----
		{
			auto lightDepthCfg = TextureConfig::GetDepthTextureConfig("Light Depth");
			std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(lightDepthCfg);
			m_TextureBuffers.push_back(depth);
			auto Func =
				[&](RenderPassBase& self) {
					RenderCommand::Clear(ClearFlags::Depth_Buffer);
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

			m_ShadowMapPass = std::move(RenderPassBuilder()
										.Name("Shadow Map")
										.Shader((shaderPathRoot / "ShadowMap.glsl").string())
										.Usage(ShaderUsage::DistantLightShadowMap)
										.AttachTarget(depth)
										.Func(Func)
										.Build());

		}

		// Gbuffers
		auto posTexCfg = TextureConfig::GetColorTextureConfig("G_Position"); posTexCfg.InternalFmt = InternalFormat::RGB16F; posTexCfg.DataFmt = DataFormat::RGB; posTexCfg.DataType = DataType::Float;
		std::shared_ptr<_Texture> position = std::make_shared<_Texture>(posTexCfg);
		m_TextureBuffers.push_back(position);

		auto normalTexCfg = TextureConfig::GetColorTextureConfig("G_Normal"); normalTexCfg.InternalFmt = InternalFormat::RGB16F; normalTexCfg.DataFmt = DataFormat::RGB; normalTexCfg.DataType = DataType::Float;
		std::shared_ptr<_Texture> normal = std::make_shared<_Texture>(normalTexCfg);
		m_TextureBuffers.push_back(normal);

		auto pbrTex = TextureConfig::GetColorTextureConfig("G_PBR"); pbrTex.InternalFmt = InternalFormat::RGB16F; pbrTex.DataFmt = DataFormat::RGB; pbrTex.DataType = DataType::Float;
		std::shared_ptr<_Texture> pbr = std::make_shared<_Texture>(pbrTex);
		m_TextureBuffers.push_back(pbr);

		std::shared_ptr<_Texture> depth = std::make_shared<_Texture>(TextureConfig::GetDepthTextureConfig("Scene Depth"));
		m_SceneDepth = depth.get();
		m_TextureBuffers.push_back(depth);

		std::shared_ptr<_Texture> baseColor = std::make_shared<_Texture>(TextureConfig::GetColorTextureConfig("G_BaseColor")); // RGBA8
		m_TextureBuffers.push_back(baseColor);

		auto depthCfg = TextureConfig::GetColorTextureConfig("DethPyramid"); depthCfg.InternalFmt = InternalFormat::R32F; depthCfg.DataFmt = DataFormat::Red; depthCfg.DataType = DataType::Float; depthCfg.GenMips = true;
		std::shared_ptr<_Texture> depthPyramid = std::make_shared<_Texture>(depthCfg);
		m_TextureBuffers.push_back(depthPyramid);
		m_SceneDepthPyramid = depthPyramid.get();

		// --- G buffer Pass --
		{
			auto Func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<VertexArrayComponent, _MaterialComponent, _TransformComponent>();
					auto shader = self.GetShader();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
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
					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};

			m_GBufferPass = std::move(RenderPassBuilder()
										.Name("G-Buffer Pass")
										.Shader((shaderPathRoot / "GBuffer.glsl").string())
										.Usage(ShaderUsage::GBuffer)
										.AttachTarget(position)
										.AttachTarget(normal)
										.AttachTarget(baseColor)
										.AttachTarget(pbr)
										.AttachTarget(depth)
										.Func(Func)
										.Build());
		}
		
		
		// --- Shading Pass ---
		{
			auto texCfg = TextureConfig::GetColorTextureConfig("Shading Result"); texCfg.InternalFmt = InternalFormat::RGBA16F; texCfg.DataFmt = DataFormat::RGBA; texCfg.DataType = DataType::Float;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			m_TextureBuffers.push_back(res);

			auto Func =
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

			m_ShadingPass = std::move(RenderPassBuilder()
				.Name("Shading Pass")
				.Shader((shaderPathRoot / "Shading.glsl").string())
				.Usage(ShaderUsage::DeferredShading)
				.AttachTarget(res)
				.Input("u_gPosition", position)
				.Input("u_gNormal", normal)
				.Input("u_gAlbedo", baseColor)
				.Input("u_gPBR", pbr)
				.Input("u_gDepth", depth)
				.Func(Func)
				.Build());
			
			m_ResultTextureID = res->GetTextureID();
			m_Result = res.get();
		}

		// --- Screen Space Reflection Pass --- 
		{


			// --- Gen Depth Pyramid Pass ---
			{
				auto Func =
					// TODO:: ?
					[depth, depthPyramid](RenderPassBase& self) {
						auto shader = self.GetShader();
						shader->Bind();
						int mipLevel = depthPyramid->GetMipLevels();
						for (int i = 0; i < mipLevel; i++) {
							// Uniform for reading
							uint32_t slot = 1;
							shader->SetInt("u_PrevMipDepth", slot);
							if (i == 0) 
								depth->BindUnit(slot);
							else
								depthPyramid->BindUnit(slot);

							shader->SetInt("u_CurrMipLevel", i);
							depthPyramid->BindTextureImage(0, i); // For writing
							int width = -1, height = -1;
							depthPyramid->GetTextureWdithHeight(width, height, i);
							AHO_CORE_ASSERT(width > 0 && height > 0);
							static int g = 16;
							int group_x = (width + g - 1) / g, group_y = (height + g - 1) / g;
							shader->DispatchCompute(group_x, group_y, 1);
						}
						shader->Unbind();
					};
				m_DepthPyramidPass = std::move(RenderPassBuilder()
					.Name("SSSR Gen Depth Pyramid Pass")
					.Shader((shaderPathRoot / "ScreenSpaceReflection" / "DepthPyramid.glsl").string())
					.Usage(ShaderUsage::GenDepthPyramid)
					.Func(Func)
					.Build());
			}

			// --- SSSR ---
			{
				RenderPassConfig cfg;
				cfg.passName = "Screen Space Reflection Pass";
				cfg.shaderPath = (shaderPathRoot / "ScreenSpaceReflection" / "SSR.glsl").string();
				auto ssrTexCfg = TextureConfig::GetColorTextureConfig("SSR"); ssrTexCfg.InternalFmt = InternalFormat::RGBA16F; ssrTexCfg.DataFmt = DataFormat::RGBA; ssrTexCfg.DataType = DataType::Float;
				std::shared_ptr<_Texture> ssrTex = std::make_shared<_Texture>(ssrTexCfg);
				m_SSRTex = ssrTex.get();

				auto Func =
					[ssrTex, depthPyramid](RenderPassBase& self) {
						auto shader = self.GetShader();
						shader->Bind();
						shader->SetInt("u_MipLevelTotal", depthPyramid->GetMipLevels());
						uint32_t slot = 1;
						self.BindRegisteredTextureBuffers(slot);
						int width = -1, height = -1;
						ssrTex->GetTextureWdithHeight(width, height, 0);
						AHO_CORE_ASSERT(width > 0 && height > 0);
						static int g = 16;
						int gx = (width + g - 1) / g;
						int gy = (height + g - 1) / g;
						ssrTex->BindTextureImage(0, 0);
						shader->DispatchCompute(gx, gy, 1);
						shader->Unbind();
					};

				m_SSRPass = std::move(RenderPassBuilder()
					.Name("SSSR Pass")
					.Shader((shaderPathRoot / "ScreenSpaceReflection" / "SSSR.glsl").string())
					.Usage(ShaderUsage::SSSR)
					.Input("u_gPosition", position)
					.Input("u_gNormal", normal)
					.Input("u_gAlbedo", baseColor)
					.Input("u_gPBR", pbr)
					.Input("u_gDepth", depthPyramid)
					.Func(Func)
					.Build());
			}
		}
	}

	void DeferredShading::Execute() {
		m_ShadowMapPass->Execute();
		m_GBufferPass->Execute();
		m_ShadingPass->Execute();
		m_DepthPyramidPass->Execute();
		m_SSRPass->Execute();
	}

	bool DeferredShading::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_GBufferPass->Resize(width, height);
		resized |= m_ShadingPass->Resize(width, height);
		resized |= m_SceneDepthPyramid->Resize(width, height);
		resized |= m_SSRTex->Resize(width, height);
		return resized;
	}

}