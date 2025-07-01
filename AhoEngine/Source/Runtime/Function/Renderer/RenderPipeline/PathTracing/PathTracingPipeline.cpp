#include "Ahopch.h"
#include "PathTracingPipeline.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/Buffer.h"
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
		constexpr int64_t MAX_MESH = 1'280'0;
		constexpr int64_t MAX_TLAS_NODE = 1'280'000;
		constexpr int64_t MAX_BLAS_NODE = 1'280'000;
		constexpr int64_t MAX_PRIMITIVE = 1'280'000;
		constexpr uint32_t MAX_PAYLOAD_SIZE = 2560 * 1440;

		struct DispatchIndirectCommand {
			GLuint num_groups_x;
			GLuint num_groups_y;
			GLuint num_groups_z;
		};
		SSBOManager::RegisterSSBO<BVHNodei>(0, MAX_TLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(1, MAX_MESH, true);
		SSBOManager::RegisterSSBO<BVHNodei>(2, MAX_BLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(3, MAX_PRIMITIVE, true);
		SSBOManager::RegisterSSBO<OffsetInfo>(4, MAX_MESH, true);
		SSBOManager::RegisterSSBO<MaterialDescriptor>(5, MAX_MESH, true);
		SSBOManager::RegisterScalar<uint32_t>(6);
		SSBOManager::RegisterSSBO<Payload>(7, MAX_PAYLOAD_SIZE, false);
		SSBOManager::RegisterSSBO<Payload>(8, MAX_PAYLOAD_SIZE, false);
		SSBOManager::RegisterSSBO<DispatchIndirectCommand>(9, 1, false);

		std::shared_ptr<_Texture> accumulateTex = TextureResourceBuilder()
			.Name("WaveFrontPathTracingAccumulate").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA32F)
			.Build();
		std::shared_ptr<_Texture> presentTex = TextureResourceBuilder()
			.Name("WaveFrontPathTracingPresent").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.Build();

		m_AccumulateTex = accumulateTex.get();
		m_PresentTex = presentTex.get();

		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "PathTracing" / "Wavefront";
		m_DispatchBuffer = std::make_shared<DispatchIndirectBuffer>(SSBOManager::GetBufferId(9));

		// --- Generate Camera Ray Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					uint32_t width = m_AccumulateTex->GetWidth();
					uint32_t height = m_AccumulateTex->GetHeight();
					shader->SetInt("u_SrcWidth", width);
					shader->SetInt("u_SrcHeight", height);
					shader->SetInt("u_WriteIndex", m_WriteIndex);

					SSBOManager::SetScalar<uint32_t>(6, width * height);

					static int group = 32;
					uint32_t workGroupCountX = (width + group - 1) / group;
					uint32_t workGroupCountY = (height + group - 1) / group;
					shader->DispatchCompute(workGroupCountX, workGroupCountY, 1);

					shader->Unbind();
				};

			m_CameraRayGenPass = std::move(RenderPassBuilder()
				.Name("CameraRayGen Pass")
				.Shader((shaderPathRoot / "RayGen.glsl").string())
				.Usage(ShaderUsage::PathTracingCamRayGen)
				.Func(Func)
				.Build());
		}

		// --- Intersection(Accumulate) Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();
					uint32_t width = m_AccumulateTex->GetWidth();
					uint32_t height = m_AccumulateTex->GetHeight();
					shader->SetInt("u_SrcWidth", width);
					shader->SetInt("u_SrcHeight", height);
					shader->SetInt("u_MaxBounce", m_MaxBounce);
					shader->SetInt("u_WriteIndex", m_WriteIndex);
					shader->SetInt("u_ReadIndex", m_ReadIndex);
					shader->SetInt("u_Frame", m_Frame);

					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					bool Reaccumulate = false;
					Reaccumulate |= SyncActiveIBLLighting(ecs, shader);

					m_AccumulateTex->BindTextureImage(0);

					// Indirect dispatch
					{
						//glMemoryBarrier(GL_COMMAND_BARRIER_BIT); // µÈ dispatch buffer ¿ÉÓÃ
						//m_DispatchBuffer->Bind();
						//glDispatchComputeIndirect(0);
						//m_DispatchBuffer->Unbind();
					}

					{
						constexpr static int group = 32;
						int gx = (width + group - 1) / group;
						int gy = (height + group - 1) / group;
						glDispatchCompute(gx, gy, 1);
					}

					shader->Unbind();
				};

			m_IntersectionPass = std::move(RenderPassBuilder()
				.Name("Intersection Pass")
				.Shader((shaderPathRoot / "Intersect.glsl").string())
				.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
			m_IntersectShader = m_IntersectionPass->GetShader();
		}

		// --- Dispatch Prep Pass ---
		{
			auto Func =
				[](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();
					shader->DispatchCompute(1, 1, 1);
					shader->Unbind();
				};

			m_DispatchPrepPass = std::move(RenderPassBuilder()
				.Name("Dispatch Prep Pass")
				.Shader((shaderPathRoot / "DispatchPrep.glsl").string())
				//.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
		}

		// --- Present Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					static int m_LastFrame = 0;
					if (m_Frame != 1 && m_LastFrame == m_Frame) {
						return; // Skip if the frame is not changed
					}
					m_LastFrame = m_Frame;
					auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
					auto shader = self.GetShader();
					shader->Bind();
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Color_Buffer);
					// Uniforms
					shader->SetInt("u_Frame", m_Frame);
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
				.Name("WaveFrontPathTracingPresent Pass")
				.Shader((shaderPathRoot / "Present.glsl").string())
				.Usage(ShaderUsage::PathTracingPresent)
				.AttachTarget(presentTex)
				.Input("u_PathTracingAccumulate", accumulateTex)
				.Func(Func)
				.Build());
		}

	}

	bool PathTracingPipeline::UpdateSceneSSBOData() const {
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;

		bool Reaccumulate = SyncSceneDirtyFlags(ecs);

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
		return Reaccumulate;
	}

	void PathTracingPipeline::Execute() {
		g_RuntimeGlobalCtx.m_Renderer->GetRenderPass("G-Buffer Pass")->Execute();
		m_CurrBounce += 1;

		bool sceneDirty = UpdateSceneSSBOData();
		bool regenCamRay = sceneDirty || m_CurrBounce > m_MaxBounce;
		if (regenCamRay) {
			//Case 1: Scene is dirty, clear texture data and begin a new accumulate process starting from frame 1
			if (sceneDirty) {
				m_AccumulateTex->ClearTextureData();
				m_CurrBounce = 1;
				m_Frame = 1;
			}
			//Case 2: Maximum bounce reached, start a new frmae
			else {
				m_CurrBounce = 1;
				m_Frame += 1;
			}
			m_WriteIndex = m_WriteIndex ^ 1;
			m_ReadIndex = m_WriteIndex ^ 1;
			m_CameraRayGenPass->Execute();
		}

		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		//m_DispatchPrepPass->Execute();

		m_WriteIndex = m_WriteIndex ^ 1;
		m_ReadIndex = m_WriteIndex ^ 1;
		AHO_CORE_ASSERT(m_WriteIndex != m_ReadIndex);
		m_IntersectionPass->Execute();

		m_PresentPass->Execute();
	}

	bool PathTracingPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_PresentPass->Resize(width, height);
		resized |= m_AccumulateTex->Resize(width, height);
		resized |= g_RuntimeGlobalCtx.m_Renderer->GetRenderPass("G-Buffer Pass")->Resize(width, height);
		return resized;
	}

	bool PathTracingPipeline::SyncActiveIBLLighting(const std::shared_ptr<EntityManager>& ecs, const Shader* shader) const {
		auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
		if (ecs->HasComponent<IBLComponent>(activeIBLEntity)) {
			auto& iblComp = ecs->GetComponent<IBLComponent>(activeIBLEntity);
			iblComp.IBL->Bind(shader);
		}
		return false;
	}

	bool PathTracingPipeline::SyncSceneDirtyFlags(const std::shared_ptr<EntityManager>& ecs) const {
		// Camera movement
		bool sceneChanged = false;
		for (auto [entity, cam] : ecs->GetView<EditorCamaraComponent>().each()) {
			if (cam.Dirty) {
				cam.Dirty = false;
				sceneChanged = true;
			}
		}

		// Material
		auto view = ecs->GetView<_BVHComponent, _TransformComponent, _MaterialComponent>();
		view.each(
			[&](auto entity, _BVHComponent& bvh, _TransformComponent& transform, _MaterialComponent& material) {
				int meshID = bvh.bvh->GetMeshId();
				if (material.Dirty) {
					material.Dirty = false; // TODO: Should not be here, but in inspector panel
					sceneChanged = true;
					material.mat.UpdateMaterialDescriptor();
					SSBOManager::UpdateSSBOData<MaterialDescriptor>(5, { material.mat.GetMatDescriptor() }, meshID);
				}
			}
		);
		return sceneChanged;

		// Objects transform
		static bool BVHDirty = true;
		std::vector<BVHi*> tasks; //tasks.reserve(view.size()); // ?
		view.each(
			[&](auto entity, _BVHComponent& bvh, _TransformComponent& transform, _MaterialComponent& material) {
				int meshID = bvh.bvh->GetMeshId();
				if (transform.Dirty) {
					transform.Dirty = false; // TODO: Should not be here, but in inspector panel
					bvh.bvh->ApplyTransform(transform.GetTransform());
					tasks.emplace_back(bvh.bvh.get());
					sceneChanged = true;
					BVHDirty = true;
				}
			}
		);

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

		return sceneChanged;
	}
}