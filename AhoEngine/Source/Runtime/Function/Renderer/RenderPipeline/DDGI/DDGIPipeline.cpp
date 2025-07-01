#include "Ahopch.h"
#include "DDGIPipeline.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/Texture/TextureResourceBuilder.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"

namespace Aho {
	DDGIPipeline::~DDGIPipeline() {
		m_ProbeVAO.reset();

		// Is this a must?
		m_ProbeIrradianceTex = nullptr;
		m_ProbeDistanceTex = nullptr;
		m_ProbeRayDataTex = nullptr;
		m_ProbeDepthTex = nullptr;
	}

	void DDGIPipeline::Initialize() {
		m_Type = RenderPipelineType::RPL_DDGI;

		// TODO: Read ddgi config; Default constructor for now
		m_Desc = DDGIVolumeDesc();

		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "DDGI";

		// --- Probe Ray Trace Pass ---
		// Only support one volume for now
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					shader->SetFloat("u_DDGIVolumeDesc.probeSpacing", m_Desc.probeSpacing);
					shader->SetUint("u_DDGIVolumeDesc.raysPerProbe", m_Desc.probeNumRays);
					shader->SetUint("u_DDGIVolumeDesc.probeCountX", m_Desc.probeCounts.x);
					shader->SetUint("u_DDGIVolumeDesc.probeCountY", m_Desc.probeCounts.y);
					shader->SetUint("u_DDGIVolumeDesc.probeCountZ", m_Desc.probeCounts.z);
					shader->SetUint("u_DDGIVolumeDesc.probeCount", m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z);

					constexpr int groups = 64;
					int numGroups = (m_Desc.probeNumRays + groups - 1) / groups;
					shader->DispatchCompute(numGroups, 1, 1);

					shader->Unbind();
				};

			m_ProbeRayTracePass = std::move(RenderPassBuilder()
				.Name("DDGIProbeRayTrace Pass")
				.Shader((shaderPathRoot / "ProbeTrace.glsl").string())
				//.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
		}

		// --- Probe Visualization Debug Pass ---
		{
			// --- Initialize a sphere vao ---
			{
				MeshOptions loadOptions((std::filesystem::current_path() / "Asset" / "Basic" / "Sphere.obj").string()); loadOptions.PreTransform = glm::mat4(1); loadOptions.BuildBVH = false; loadOptions.IsSkeletalMesh = false;
				auto& ecs = g_RuntimeGlobalCtx.m_EntityManager;
				auto resourceManager = g_RuntimeGlobalCtx.m_Resourcemanager;
				std::shared_ptr<MeshAsset> ms = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<MeshAsset>(ecs, loadOptions);
				m_ProbeVAO = resourceManager->LoadVAO(ms);
			}

			auto probeVisualTexture = TextureResourceBuilder()
				.Name("DDGIProbeVisualTexture").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA32F)
				.Build();
			auto probeVisualDepthTexture = TextureResourceBuilder()
				.Name("DDGIProbeDepth").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::Depth).InternalFormat(InternalFormat::Depth32F)
				.Build();
			m_ProbeVisualTex = probeVisualTexture.get();

			auto Func =
				[this](RenderPassBase& self) {
					self.GetRenderTarget()->Bind();
					auto shader = self.GetShader();
					shader->Bind();
					m_ProbeVAO->Bind();
					RenderCommand::DrawIndexed(m_ProbeVAO.get());
					m_ProbeVAO->Unbind();
					shader->Unbind();
					self.GetRenderTarget()->Unbind();
				};

			m_ProbeVisualizationPass = std::move(RenderPassBuilder()
				.Name("DDGIProbeVisual Pass")
				.Shader((shaderPathRoot / "ProbeVisual.glsl").string())
				.AttachTarget(probeVisualTexture)
				.AttachTarget(probeVisualDepthTexture)
				//.Usage(ShaderUsage::DDGIProbeVisual)
				.Func(Func)
				.Build());
		}

		// --- Composite Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();
					shader->Unbind();
				};

			m_CompositePass = std::move(RenderPassBuilder()
				.Name("DDGIComposite Pass")
				.Shader((shaderPathRoot / "Composite.glsl").string())
				//.Usage(ShaderUsage::DDGIComposite)
				.Func(Func)
				.Build());
		}
	}

	void DDGIPipeline::Execute() {
		m_ProbeRayTracePass->Execute();
	}

	bool DDGIPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_ProbeVisualTex->Resize(width, height);
		resized |= m_ProbeVisualizationPass->Resize(width, height);
		return resized;
	}


}