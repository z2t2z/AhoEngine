#include "Ahopch.h"
#include "PathTracingPipeline.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"
#include "Runtime/Function/Renderer/Texture/TextureResourceBuilder.h"
#include "Runtime/Function/Renderer/IBL/IBLManager.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"

namespace Aho {
	PathTracingPipeline::PathTracingPipeline() {
		Initialize();
	}

	// TODO: Wave front, blas does not update per frame
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

		std::shared_ptr<_Texture> accumulateTex = TextureResourceBuilder()
			.Name("PathTracingAccumulate").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA32F)
			.Build();
		std::shared_ptr<_Texture> presentTex = TextureResourceBuilder()
			.Name("PathTracingPresent").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();

		m_TextureBuffers.push_back(accumulateTex);
		m_TextureBuffers.push_back(presentTex);

		m_Result = presentTex.get();
		
		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "PathTracing";
		// --- Accumulate pass ---
		{
			auto Func =
				[accumulateTex, this](RenderPassBase& self) {
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

					static bool BVHDirty = true;
					auto view = ecs->GetView<_BVHComponent, _TransformComponent, _MaterialComponent>();
					std::vector<BVHi*> tasks; //tasks.reserve(view.size()); // ?
					view.each(
						[&](auto entity, _BVHComponent& bvh, _TransformComponent& transform, _MaterialComponent& material) {
							int meshID = bvh.bvh->GetMeshId();
							if (transform.Dirty) {
								transform.Dirty = false; // TODO: Should not be here, but in inspector panel
								bvh.bvh->ApplyTransform(transform.GetTransform());
								tasks.emplace_back(bvh.bvh.get());
								Reaccumulate = true;
								BVHDirty = true;
							}
							if (material.Dirty) {
								material.Dirty = false; // TODO: Should not be here, but in inspector panel
								Reaccumulate = true;
								material.mat.UpdateMaterialDescriptor();
								SSBOManager::UpdateSSBOData<MaterialDescriptor>(5, { material.mat.GetMatDescriptor() }, meshID);
							}
						}
					);
					{
						const auto& executor = g_RuntimeGlobalCtx.m_ParallelExecutor;
						size_t siz = tasks.size();
						g_RuntimeGlobalCtx.m_ParallelExecutor->ParallelFor(siz,
							[tasks](int64_t i) {
								tasks[i]->Rebuild();
							}, 1
						);
					}
					// No need to update every blas everytime but for now it's fine
					if (BVHDirty) {
						BVHDirty = false;
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

					if (Reaccumulate) {
						m_Frame = 1;
						accumulateTex->ClearTextureData();
					}
					accumulateTex->BindTextureImage(0);
					shader->SetInt("u_Frame", m_Frame);
					static int group = 16;
					uint32_t width = accumulateTex->GetWidth();
					uint32_t height = accumulateTex->GetHeight();
					uint32_t workGroupCountX = (width + group - 1) / group;
					uint32_t workGroupCountY = (height + group - 1) / group;
					shader->DispatchCompute(workGroupCountX, workGroupCountY, 1);
					shader->Unbind();
				};

			m_AccumulatePass = std::move(RenderPassBuilder()
				.Name("PathTracingAccumulate Pass")
				.Shader((shaderPathRoot / "PathTracing.glsl").string())
				.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
		}

		// --- Present Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
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

			m_PresentPass = std::move(RenderPassBuilder()
				.Name("PathTracingPresent Pass")
				.Shader((shaderPathRoot / "Present.glsl").string())
				//.Usage(ShaderUsage::PathTracing)
				.AttachTarget(presentTex)
				.Input("u_PathTracingAccumulate", accumulateTex)
				.Func(Func)
				.Build());
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