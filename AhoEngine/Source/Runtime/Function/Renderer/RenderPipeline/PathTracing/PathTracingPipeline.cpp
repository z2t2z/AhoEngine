#include "Ahopch.h"
#include "PathTracingPipeline.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureUsage.h"
#include "Runtime/Function/Renderer/Texture/TextureConfig.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/IBL/IBLManager.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Core/Timer.h"

namespace Aho {
	PathTracingPipeline::PathTracingPipeline() {
		Initialize();
	}

	void PathTracingPipeline::Initialize() {
		constexpr int64_t MAX_MESH		= 1'280'0;
		constexpr int64_t MAX_TLAS_NODE = 1'280'000;
		constexpr int64_t MAX_BLAS_NODE = 1'280'000;
		constexpr int64_t MAX_PRIMITIVE = 1'280'000;

		SSBOManager::RegisterSSBO<BVHNodei>(0, MAX_TLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(1, MAX_MESH, true);
		SSBOManager::RegisterSSBO<BVHNodei>(2, MAX_BLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(3, MAX_PRIMITIVE, true);
		SSBOManager::RegisterSSBO<OffsetInfo>(4, MAX_MESH, true);
		SSBOManager::RegisterSSBO<MaterialDescriptor>(5, MAX_MESH, true);


		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "PathTracing";

		// --- Accumulate pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Path Tracing Accumulate Pass";
			cfg.shaderPath = (shaderPathRoot / "PathTracing.glsl").string();

			TextureConfig texCfg = TextureConfig::GetColorTextureConfig("PathTracingAccumulate");
			texCfg.InternalFmt = InternalFormat::RGBA32F;
			texCfg.DataType = DataType::Float;
			texCfg.Width = 1280; texCfg.Height = 720;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			m_TextureBuffers.push_back(res);
			cfg.textureAttachments.push_back(res.get());

			cfg.func =
				[&](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto camView = ecs->GetView<EditorCamaraComponent>();
					bool Reaccumulate = false;
					camView.each(
						[&](auto cam, EditorCamaraComponent& cmp) { 
							if (cmp.Dirty) {
								cmp.Dirty = false;
								Reaccumulate = true;
							}
						});
					auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
					if (ecs->HasComponent<IBLComponent>(activeIBLEntity)) {
						auto& iblComp = ecs->GetComponent<IBLComponent>(activeIBLEntity);
						iblComp.IBL->Bind(shader);
					}

					bool BVHDirty = false;
					auto view = ecs->GetView<_BVHComponent, _TransformComponent, _MaterialComponent>();
					view.each(
						[&](auto entity, _BVHComponent& bvh, _TransformComponent& transform, _MaterialComponent& material) {
							int meshID = bvh.bvh->GetMeshId();
							if (bvh.Dirty) {
								bvh.Dirty = false;
								{
									ScopedTimer timer("ApplyTransform");
									bvh.bvh->ApplyTransform(transform.GetTransform());
								}
								Reaccumulate = true;
								BVHDirty = true;
							}
							if (material.Dirty) {
								material.Dirty = false;
								Reaccumulate = true;
								SSBOManager::UpdateSSBOData<MaterialDescriptor>(5, { material.mat.GetMatDescriptor() }, meshID);
							}
						}
					);
					// No need to update every blas everytime but for now it's fine
					if (BVHDirty) {
						auto sceneView = ecs->GetView<SceneBVHComponent>();
						AHO_CORE_ASSERT(sceneView.size() <= 1);
						sceneView.each(
							[&](auto entity, SceneBVHComponent& sceneBvh) {
								sceneBvh.bvh->UpdateTLAS();
								const auto& tlas = sceneBvh.bvh;
								SSBOManager::UpdateSSBOData<BVHNodei>(0, tlas->GetNodesArr());
								SSBOManager::UpdateSSBOData<PrimitiveDesc>(1, tlas->GetPrimsArr());
								SSBOManager::UpdateSSBOData<OffsetInfo>(4, tlas->GetOffsetMap());

								size_t nodesOffset = 0;
								size_t primsOffset = 0;
								const std::vector<OffsetInfo>& info = tlas->GetOffsetMap();
								for (size_t i = 0; i < tlas->GetPrimsCount(); i++) {
									const BVHi* blas = tlas->GetBLAS(i);
									AHO_CORE_ASSERT(nodesOffset == info[i].nodeOffset);
									AHO_CORE_ASSERT(primsOffset == info[i].primOffset);
									SSBOManager::UpdateSSBOData<BVHNodei>(2, blas->GetNodesArr(), nodesOffset);
									SSBOManager::UpdateSSBOData<PrimitiveDesc>(3, blas->GetPrimsArr(), primsOffset);
									nodesOffset += blas->GetNodeCount();
									primsOffset += blas->GetPrimsCount();
								}
							}
						);
					}

					auto textureTarget = self.GetTextureAttachmentByIndex(0);
					if (Reaccumulate) {
						static constexpr float clearData[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						m_Frame = 1;
						textureTarget->ClearTextureData(clearData);
					}
					textureTarget->BindTextureImage(0);
					shader->SetInt("u_Frame", m_Frame);
					static int group = 16;
					uint32_t width = textureTarget->GetWidth();
					uint32_t height = textureTarget->GetHeight();
					uint32_t workGroupCountX = (width + group - 1) / group;
					uint32_t workGroupCountY = (height + group - 1) / group;
					shader->DispatchCompute(workGroupCountX, workGroupCountY, 1);
					shader->Unbind();
				};
			m_AccumulatePass = std::make_unique<RenderPassBase>();
			m_AccumulatePass->Setup(cfg);
		}

		// --- Present Pass ---
		{
			RenderPassConfig cfg;
			cfg.passName = "Path Tracing Present Pass";
			cfg.shaderPath = (shaderPathRoot / "Present.glsl").string();

			auto texCfg = TextureConfig::GetColorTextureConfig("PathTracingPresent");
			texCfg.InternalFmt = InternalFormat::RGBA16F; // Use HDR format for shading result
			texCfg.DataFmt = DataFormat::RGBA;
			texCfg.DataType = DataType::Float;
			std::shared_ptr<_Texture> res = std::make_shared<_Texture>(texCfg);
			m_TextureBuffers.push_back(res);
			cfg.textureAttachments.push_back(res.get());

			cfg.func =
				[&](RenderPassBase& self) {
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto shader = self.GetShader();
					shader->Bind();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);

					// Uniforms
					shader->SetInt("u_Frame", m_Frame++);

					// Texture uniforms
					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);
					
					// Draw screen quad
					glBindVertexArray(self.s_DummyVAO); // Draw a screen quad for shading
					RenderCommand::DrawArray();

					shader->Unbind();
					self.GetRenderTarget()->Unbind();
				};
			m_PresentPass = std::make_unique<RenderPassBase>();
			m_PresentPass->Setup(cfg);
			m_PresentPass->RegisterTextureBuffer(m_AccumulatePass->GetTextureAttachmentByIndex(0), "u_PathTracingAccumulate");
			m_ResultTextureID = res->GetTextureID();
			m_Result = res.get();
		}
	}

	void PathTracingPipeline::Execute() {
		m_AccumulatePass->Execute();
		m_PresentPass->Execute();
	}

	bool PathTracingPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = m_AccumulatePass->Resize(width, height);
		resized |= m_PresentPass->Resize(width, height);
		return resized;
	}


}