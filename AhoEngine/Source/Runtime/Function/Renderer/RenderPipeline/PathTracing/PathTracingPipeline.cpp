#include "Ahopch.h"
#include "PathTracingPipeline.h"
#include "Runtime/Function/Level/Level.h"
#include "Runtime/Core/BVH.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include <memory>
#include <execution>

namespace Aho {
	static std::filesystem::path g_CurrentPath = std::filesystem::current_path();

	PathTracingPipeline::PathTracingPipeline() {
		Initialize();
		m_Frame = 1u;
	}

	void PathTracingPipeline::Initialize() {
		// TODO: Move these and check them in bvh
		constexpr int64_t MAX_MESH		= 1'280'000;
		constexpr int64_t MAX_TLAS_NODE = 1'280'000;
		constexpr int64_t MAX_BLAS_NODE = 1'280'000;
		constexpr int64_t MAX_PRIMITIVE = 1'280'000;

		SSBOManager::RegisterSSBO<BVHNodei>(0, MAX_TLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(1, MAX_MESH, true);
		SSBOManager::RegisterSSBO<BVHNodei>(2, MAX_BLAS_NODE, true);
		SSBOManager::RegisterSSBO<PrimitiveDesc>(3, MAX_PRIMITIVE, true);
		SSBOManager::RegisterSSBO<OffsetInfo>(4, MAX_MESH, true);
		SSBOManager::RegisterSSBO<TextureHandles>(5, MAX_MESH, true);

		m_AccumulatePass = SetupAccumulatePass();

		m_AccumulatePass->RegisterTextureBuffer({ m_AccumulatePass->GetTextureBuffer(TexType::PathTracingAccumulate), TexType::PathTracingAccumulate });

		RegisterRenderPass(m_AccumulatePass.get(), RenderDataType::ScreenQuad);
		m_RenderResult = m_AccumulatePass->GetTextureBuffer(TexType::Result);
	}

	void PathTracingPipeline::UpdateSSBO(const std::shared_ptr<Level>& currLevel) {
		const auto& tlas = currLevel->GetTLAS();
		SSBOManager::UpdateSSBOData<BVHNodei>(0, tlas.GetNodesArr());
		SSBOManager::UpdateSSBOData<PrimitiveDesc>(1, tlas.GetPrimsArr());
		SSBOManager::UpdateSSBOData<OffsetInfo>(4, tlas.GetOffsetMap());

		size_t nodesOffset = 0;
		size_t primsOffset = 0;
		int i = 0;
		std::vector<OffsetInfo> info = tlas.GetOffsetMap();
		for (const PrimitiveDesc& blasPrim : tlas.GetPrimsArr()) {
			const BVHi* blas = tlas.GetBLAS(blasPrim.GetPrimId());
			AHO_CORE_ASSERT(nodesOffset == info[i].nodeOffset);
			AHO_CORE_ASSERT(primsOffset == info[i].primOffset);

			SSBOManager::UpdateSSBOData<BVHNodei>(2, blas->GetNodesArr(), nodesOffset);
			SSBOManager::UpdateSSBOData<PrimitiveDesc>(3, blas->GetPrimsArr(), primsOffset);

			nodesOffset += blas->GetNodeCount();
			primsOffset += blas->GetPrimsCount();
			i += 1;
		}
	}

	void PathTracingPipeline::ClearAccumulateData() {
		m_Frame = 1u;
		static constexpr float clearData[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_AccumulatePass->GetRenderTarget()->GetTexture(TexType::PathTracingAccumulate)->ClearTexImage(clearData);
	}

	static Texture* s_EnvMap = nullptr;
	void PathTracingPipeline::SetEnvMap(Texture* texture) {
		s_EnvMap = texture;

		m_IBL = new IBL(texture);

	}

	bool PathTracingPipeline::ResizeRenderTarget(uint32_t width, uint32_t height) {
		bool resized = false;
		for (auto& task : m_RenderTasks) {
			resized |= task.pass->GetRenderTarget()->Resize(width, height);
		}
		if (resized) {
			m_Frame = 1u;
		}
		return resized;
	}

	constexpr uint32_t g_EnvMapBindingPoint = 0;
	std::unique_ptr<RenderPass> PathTracingPipeline::SetupAccumulatePass() {
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("PathTracingPass");
		// Accumulate in compute shader
		pass->AddRenderCommand(
			[this](const std::vector<std::shared_ptr<RenderData>>& _, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				static int g = 32;
				shader->Bind();
				renderTarget->BindAt(0, 0);

				shader->SetInt("u_Frame", m_Frame++);
				if (s_EnvMap) {
					shader->SetInt("u_EnvLight", g_EnvMapBindingPoint);
					s_EnvMap->Bind(g_EnvMapBindingPoint);
				}

				if (m_IBL) {
					shader->SetInt("u_EnvMap.EnvLight", g_EnvMapBindingPoint + 1);
					shader->SetInt("u_EnvMap.Env1DCDF", g_EnvMapBindingPoint + 2);
					shader->SetInt("u_EnvMap.Env2DCDF", g_EnvMapBindingPoint + 3);
					shader->SetInt("u_EnvMap.Env2DCDF_Reference", g_EnvMapBindingPoint + 4);

					m_IBL->BindEnvMap(g_EnvMapBindingPoint + 1);
					m_IBL->Bind1DCDF(g_EnvMapBindingPoint + 2);
					m_IBL->Bind2DCDF(g_EnvMapBindingPoint + 3);
					m_IBL->Bind2DCDFReference(g_EnvMapBindingPoint + 4);
					shader->SetIvec2("u_EnvMap.EnvSize", glm::ivec2(m_IBL->GetSize()));
					shader->SetFloat("u_EnvMap.EnvTotalLum", m_IBL->GetTotLuminance());
				}

				uint32_t width = renderTarget->GetSpecification().Width;
				uint32_t height = renderTarget->GetSpecification().Height;
				uint32_t workGroupCountX = (width + g - 1) / g;
				uint32_t workGroupCountY = (height + g - 1) / g;
				shader->DispatchCompute(workGroupCountX, workGroupCountY, 1);

				renderTarget->Unbind();
				shader->Unbind();
			});

		// Present in fragment shader
		pass->AddRenderCommand(
			[this](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->EnableAttachments(1);
				RenderCommand::Clear(ClearFlags::Color_Buffer);

				shader->SetInt("u_Frame", m_Frame++);

				uint32_t texOffset = 0u;
				for (const auto& texBuffer : textureBuffers) {
					shader->SetInt(TextureBuffer::GetTexBufferUniformName(texBuffer.m_Type), texOffset);
					texBuffer.m_Texture->Bind(texOffset++);
				}

				for (const auto& data : renderData) {
					data->Bind(shader, texOffset);
					RenderCommand::DrawIndexed(data->GetVAO());
					data->Unbind();
				}
				renderTarget->Unbind();
				shader->Unbind();
			});

		auto accumulateShader = Shader::Create((g_CurrentPath / "ShaderSrc" / "PathTracing" / "PathTracing.glsl").string());
		AHO_CORE_ASSERT(accumulateShader->IsCompiled());
		pass->AddShader(accumulateShader);

		auto preSentShader = Shader::Create((g_CurrentPath / "ShaderSrc" / "PathTracing" / "Present.glsl").string());
		AHO_CORE_ASSERT(preSentShader->IsCompiled());
		pass->AddShader(preSentShader);


		TexSpec colorAttachment;
		colorAttachment.debugName = "pathTracerResult";
		colorAttachment.internalFormat = TexInterFormat::RGBA32F;
		colorAttachment.dataFormat = TexDataFormat::RGBA;
		colorAttachment.dataType = TexDataType::Float;
		colorAttachment.filterModeMin = TexFilterMode::Nearest;
		colorAttachment.filterModeMag = TexFilterMode::Nearest;
		colorAttachment.type = TexType::Result;

		TexSpec colorAttachmentAccumulate(colorAttachment);
		colorAttachmentAccumulate.type = TexType::PathTracingAccumulate;
		colorAttachmentAccumulate.debugName = "pathTracerAccumulate";

		FBSpec fbSpec(1280u, 720u, { colorAttachmentAccumulate, colorAttachment });

		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::PathTracing);
		return pass;
	}

	std::unique_ptr<RenderPass> PathTracingPipeline::SetupGBufferPass() {
		std::unique_ptr<RenderCommandBuffer> cmdBuffer = std::make_unique<RenderCommandBuffer>();
		cmdBuffer->AddCommand(
			[](const std::vector<std::shared_ptr<RenderData>>& renderData, const std::shared_ptr<Shader>& shader, const std::vector<TextureBuffer>& textureBuffers, const std::shared_ptr<Framebuffer>& renderTarget) {
				shader->Bind();
				renderTarget->Bind();
				RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);

				for (const auto& data : renderData) {
					if (!data->ShouldBeRendered()) {
						continue;
					}
					data->Bind(shader);

					shader->SetUint("u_EntityID", data->GetEntityID());
					if (RendererGlobalState::g_SelectedEntityID == data->GetEntityID()) {
						RendererGlobalState::g_SelectedData = data;
					}

					if (data->IsInstanced()) {
						shader->SetBool("u_IsInstanced", true);
						RenderCommand::DrawIndexedInstanced(data->GetVAO(), data->GetVAO()->GetInstanceAmount());
						shader->SetBool("u_IsInstanced", false);
					}
					else {
						RenderCommand::DrawIndexed(data->GetVAO());
					}
					data->Unbind();
				}

				renderTarget->Unbind();
				shader->Unbind();
			});

		auto shader = Shader::Create((g_CurrentPath / "ShaderSrc" / "SSAO_GeoPass.glsl").string());
		std::unique_ptr<RenderPass> pass = std::make_unique<RenderPass>("GbufferPass");
		AHO_CORE_ASSERT(shader->IsCompiled());
		pass->SetShader(shader);
		pass->SetRenderCommand(std::move(cmdBuffer));
		TexSpec depth; depth.internalFormat = TexInterFormat::Depth24; depth.dataFormat = TexDataFormat::DepthComponent; depth.type = TexType::Depth; depth.debugName = "depth";

		TexSpec positionAttachment;
		positionAttachment.debugName = "position";
		positionAttachment.internalFormat = TexInterFormat::RGBA16F;
		positionAttachment.dataFormat = TexDataFormat::RGBA;
		positionAttachment.dataType = TexDataType::Float;
		positionAttachment.filterModeMin = TexFilterMode::Nearest;
		positionAttachment.filterModeMag = TexFilterMode::Nearest;
		positionAttachment.type = TexType::Position;

		TexSpec normalAttachment = positionAttachment;
		normalAttachment.debugName = "normal";
		normalAttachment.wrapModeS = TexWrapMode::None;
		normalAttachment.type = TexType::Normal;

		TexSpec albedoAttachment;
		albedoAttachment.debugName = "albedo";
		albedoAttachment.type = TexType::Albedo;

		TexSpec entityAttachment;
		entityAttachment.debugName = "entity";
		entityAttachment.internalFormat = TexInterFormat::UINT;
		entityAttachment.dataFormat = TexDataFormat::UINT;
		entityAttachment.dataType = TexDataType::UnsignedInt;
		entityAttachment.type = TexType::Entity;

		TexSpec pbrAttachment = positionAttachment;
		pbrAttachment.debugName = "pbr";
		pbrAttachment.internalFormat = TexInterFormat::RGBA16F; pbrAttachment.dataFormat = TexDataFormat::RGBA; // Don't use rbga8 cause it will be normalized!
		pbrAttachment.dataType = TexDataType::Float;
		pbrAttachment.type = TexType::PBR;

		FBSpec fbSpec(1280u, 720u, { positionAttachment, normalAttachment, albedoAttachment, pbrAttachment, entityAttachment, depth });

		auto fbo = Framebuffer::Create(fbSpec);
		pass->SetRenderTarget(fbo);
		pass->SetRenderPassType(RenderPassType::SSAOGeo);
		return pass;
	}

}