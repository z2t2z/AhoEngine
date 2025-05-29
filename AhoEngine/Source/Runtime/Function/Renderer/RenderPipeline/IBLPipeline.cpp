#include "Ahopch.h"
#include "IBLPipeline.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureUsage.h"
#include "Runtime/Function/Renderer/Texture/TextureConfig.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

namespace Aho {
	void _IBLPipeline::Initialize() {
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "IBL";

		// --- Generate cubemap from equirectangular map --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "Generate Cubemap From Equirectangular Map Pass";
			cfg.shaderPath = (shaderPathRoot / "EquirectToCube.glsl").string();

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* cubeMap = nullptr;
					_Texture* equiRec = nullptr;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.EnvTextureSkyBox) {
								iblComp.EnvTextureSkyBox = std::make_unique<_Texture>(TextureConfig::GetCubeMapTextureConfig());
								cubeMap = iblComp.EnvTextureSkyBox.get();
								equiRec = iblComp.EnvTexture;
							}
						}
					);
					if (!equiRec) {
						return;
					}

					// Write to cube map
					cubeMap->BindTextureImage(0);

					// Read from equirectangular map
					RenderCommand::BindTextureUnit(0, equiRec->GetTextureID());
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_EquirectangularMap", 0);
					shader->SetVec2("u_EquirectangularMapSize", { equiRec->GetWidth(), equiRec->GetHeight() });
					shader->SetFloat("u_CubemapFaceSize", cubeMap->GetWidth());

					static int group = 16;
					uint32_t face = cubeMap->GetWidth();
					uint32_t numGroups = (face + group - 1) / group;
					shader->DispatchCompute(numGroups, numGroups, 6);
					shader->Unbind();

					cubeMap->GenMipMap();
				};
			m_RP_GenCubemapFromSphericalMap = std::make_unique<RenderPassBase>();
			m_RP_GenCubemapFromSphericalMap->Setup(cfg);
		}

		// --- Precompute Irradiance Map Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Precompute Irradiance Map Pass";
			cfg.shaderPath = (shaderPathRoot / "IrradianceMap.glsl").string();
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* skyboxEnvMap = nullptr;
					_Texture* irrandianceMap = nullptr;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.Irradiance) {
								auto texCfg = TextureConfig::GetCubeMapTextureConfig("Irradiance");
								texCfg.Height = texCfg.Width = 64;
								iblComp.Irradiance = std::make_unique<_Texture>(texCfg);
								irrandianceMap = iblComp.Irradiance.get();
								skyboxEnvMap = iblComp.EnvTextureSkyBox.get();
							}
						}
					);
					if (!skyboxEnvMap) {
						return;
					}
					// Write to irrandiance Map
					irrandianceMap->BindTextureImage(0);

					// Read from skyboxEnvMap(cubemap)
					RenderCommand::BindTextureUnit(0, skyboxEnvMap->GetTextureID());
					
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_gCubeMap", 0);
					static int group = 16;
					uint32_t face = irrandianceMap->GetWidth();
					uint32_t numGroups = (face + group - 1) / group;
					shader->DispatchCompute(numGroups, numGroups, 6);
					shader->Unbind();
				};
			m_RP_PrecomputeIrradiance = std::make_unique<RenderPassBase>();
			m_RP_PrecomputeIrradiance->Setup(cfg);
		}

		// --- Prefilter Map Pass --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "Precompute Prefilter Map Pass";
			cfg.shaderPath = (shaderPathRoot / "Prefiltering.glsl").string();
			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto view = ecs->GetView<IBLComponent>();
					_Texture* skyboxEnvMap = nullptr;
					_Texture* prefilterMap = nullptr;
					int prefilterBaseRes = 512;
					view.each(
						[&](auto _, IBLComponent& iblComp) {
							if (!iblComp.Prefilter) {
								auto texCfg = TextureConfig::GetCubeMapTextureConfig("Prefilter");
								texCfg.GenMips = true;
								texCfg.Height = texCfg.Width = iblComp.PrefilterBaseRes;
								prefilterBaseRes = iblComp.PrefilterBaseRes;
								iblComp.Prefilter = std::make_unique<_Texture>(texCfg);
								prefilterMap = iblComp.Prefilter.get();
								skyboxEnvMap = iblComp.EnvTextureSkyBox.get();
							}
						}
					);
					if (!skyboxEnvMap) {
						return;
					}

					// Read from envMap(cubemap)
					RenderCommand::BindTextureUnit(0, skyboxEnvMap->GetTextureID());
					auto shader = self.GetShader();
					shader->Bind();
					shader->SetInt("u_gCubeMap", 0);

					static int group = 16;
					int maxMipLevels = int(log2(prefilterBaseRes)) + 1;
					for (int mip = 0; mip < maxMipLevels; mip++) {
						// Write to prefilter Map
						prefilterMap->BindTextureImage(0, mip);
						int mipSize = prefilterBaseRes >> mip;
						float roughness = (float)mip / (float)(maxMipLevels - 1);
						shader->SetFloat("u_Roughness", roughness);
						shader->SetFloat("u_Size", mipSize);
						shader->SetInt("u_MipLevel", mip);
						int numGroups = (mipSize + group - 1) / group;
						shader->DispatchCompute(numGroups, numGroups, 6);
					}

					shader->Unbind();
				};
			m_RP_Prefiltering = std::make_unique<RenderPassBase>();
			m_RP_Prefiltering->Setup(cfg);
		}

		// --- BRDF Integrate Pass --- 
		{
			RenderPassConfig cfg;
			cfg.passName = "BRDF Integrate Pass";
			cfg.shaderPath = (shaderPathRoot / "BRDFIntegrate.glsl").string();
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
			m_RP_BRDFLUT = std::make_unique<RenderPassBase>();
			m_RP_BRDFLUT->Setup(cfg);
		}
	}

	void _IBLPipeline::Execute() {
		m_RP_GenCubemapFromSphericalMap->Execute();
		m_RP_PrecomputeIrradiance->Execute();
		m_RP_Prefiltering->Execute();
		m_RP_BRDFLUT->Execute();
	}
}
