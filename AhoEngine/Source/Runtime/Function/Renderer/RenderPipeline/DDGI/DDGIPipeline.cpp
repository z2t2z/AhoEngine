#include "Ahopch.h"
#include "DDGIPipeline.h"
#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/Texture/TextureResourceBuilder.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPassBuilder.h"

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/normalize_dot.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>


namespace Aho {
	DDGIPipeline::~DDGIPipeline() {
		m_ProbeDepthTex = nullptr;
	}

	inline float saturate(float x) {
		return glm::clamp(x, 0.0f, 1.0f);
	}
	// 等价于 HLSL 的 lerp(a, b, t)
	inline glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, const glm::bvec2& t) {
		return glm::mix(a, b, glm::vec2(t));
	}
	inline float SignNotZero(float v) {
		return v >= 0.0f ? 1.0f : -1.0f;
	}

	inline glm::vec2 SignNotZero(const glm::vec2& v) {
		return glm::vec2(SignNotZero(v.x), SignNotZero(v.y));
	}
	glm::vec2 EncodeNormalOctahedron(glm::vec3 direction) {
		// Y-up: 把 Y 作为上方向，方向改为 (x, z, y)
		glm::vec3 dirXZUp(direction.x, direction.z, direction.y);

		float l1norm = std::abs(dirXZUp.x) + std::abs(dirXZUp.y) + std::abs(dirXZUp.z);
		//glm::vec2 uv = dirXZUp.xy / l1norm;
		glm::vec2 uv = glm::vec2(dirXZUp.x, dirXZUp.y) / l1norm; // 只取 x 和 y 分量

		if (dirXZUp.z < 0.0f) {
			uv = (1.0f - glm::abs(glm::vec2(uv.y, uv.x))) * SignNotZero(uv);
		}

		return uv;
	}

	glm::vec3 DecodeNormalOctahedron(glm::vec2 uv) {
		glm::vec3 dir(uv.x, uv.y, 1.0f - std::abs(uv.x) - std::abs(uv.y));

		if (dir.z < 0.0f) {
			float oldX = dir.x;
			dir.x = (1.0f - std::abs(dir.y)) * SignNotZero(oldX);
			dir.y = (1.0f - std::abs(oldX)) * SignNotZero(dir.y);
		}

		dir = glm::normalize(dir);

		// Y-up: 把方向从 (x, z, y) 还原为 (x, y, z)
		return glm::vec3(dir.x, dir.z, dir.y);
	}

	static void test() {
		for (int i = 0; i < 7; i++) {
			for (int j = 0; j < 7; j++) {
				glm::vec2 uv = glm::vec2(float(i) + 0.5f, float(j) + 0.5f) / glm::vec2(7);

				glm::vec2 f = uv * 2.0f - 1.0f;

				glm::vec3 dir  = DecodeNormalOctahedron(f);
				glm::vec2 uv1  = EncodeNormalOctahedron(dir);
				glm::vec2 f1   = uv1;
				glm::vec3 dir1 = DecodeNormalOctahedron(f1);
				
				AHO_CORE_TRACE("uv  : ({}, {})", uv.x, uv.y);
				AHO_CORE_TRACE("f   : ({}, {})", f.x, f.y);
				AHO_CORE_TRACE("Dir :({},{},{})", dir.x, dir.y, dir.z);
				AHO_CORE_TRACE("f1  : ({}, {})", f1.x, f1.y);
				AHO_CORE_TRACE("Dir1:({},{},{})\n", dir1.x, dir1.y, dir1.z);


				//AHO_CORE_TRACE("Original uv: ({}, {})", uv.x, uv.y);
				//AHO_CORE_TRACE("Encoded  uv: ({}, {})\n", uv1.x, uv1.y);
				//AHO_CORE_TRACE("Dir0:({},{},{})", dir.x, dir.y, dir.z);
				//AHO_CORE_TRACE("Dir1:({},{},{})", dir1.x, dir1.y, dir1.z);
			}
		}

		for (int i = 0; i < 20; i++) {
			glm::vec2 randomUV = glm::vec2(float(rand() % 1000) / 1000.0f, float(rand() % 1000) / 1000.0f);
			randomUV = glm::clamp(randomUV, 0.0f, 1.0f);
			glm::vec3 dir = DecodeNormalOctahedron(randomUV);
			glm::vec2 uv1 = EncodeNormalOctahedron(dir);
			AHO_CORE_TRACE("Random  UV: ({}, {})", randomUV.x, randomUV.y);
			AHO_CORE_TRACE("Encoded UV: ({}, {})", uv1.x, uv1.y);
		}
	}

	void test1(const glm::vec3 probeCounts) {
		int total = probeCounts.x * probeCounts.y * probeCounts.z;
		for (int i = 0; i < total; i++) {
			int width = probeCounts.x;
			int height = probeCounts.y;
			int depth = probeCounts.z;

			int probeIndex = i;
			int x = probeIndex % width;
			int y = probeIndex / (width * depth);
			int z = (probeIndex % (width * depth)) / width;
			glm::vec3 pos = glm::vec3(x, y, z);
			AHO_CORE_TRACE("Probe {}: Coords: ({}, {}, {})", i, pos.x, pos.y, pos.z);
		}
	}

	void DDGIPipeline::Initialize() {
		//test();
		//test1(glm::vec3(2, 3, 4));
		m_Type = RenderPipelineType::RPL_DDGI;

		// TODO: Read ddgi config; Default constructor for now
		m_Desc = DDGIVolumeDesc();

		m_Desc.origin = glm::vec3(-10, -1, -5);
		m_Desc.probeCounts = glm::ivec3(24, 12, 14);
		
		//m_Desc.probeCounts = glm::ivec3(2, 3, 4);
		m_Desc.probeSpacing = 1.0;
		m_Desc.raysPerProbe = 64;

		std::filesystem::path shaderPathRoot = std::filesystem::current_path() / "ShaderSrc" / "DDGI";

		// Buffers
		int probeCounts = m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z;
		int y = m_Desc.probeCounts.y;
		int xz = m_Desc.probeCounts.x * m_Desc.probeCounts.z;

		m_DDGIProbeIrradiance = TextureResourceBuilder()
			.Name("DDGIProbeIrradiance0").Width(xz * (PROBE_IRRADIANCE_TEXELS + 2)).Height(y * (PROBE_IRRADIANCE_TEXELS + 2)).Dimension(TextureDim::Texture2D)
			.DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.WrapModeS(WrapMode::Repeat).WrapModeT(WrapMode::Repeat)
			.FilterMin(Filtering::Linear).FilterMag(Filtering::Linear)
			.Build();

		m_DDGIProbeDistance = TextureResourceBuilder()
			.Name("DDGIProbeDistance0").Width(xz * (PROBE_DISTANCE_TEXELS + 2)).Height(y * (PROBE_DISTANCE_TEXELS)).Dimension(TextureDim::Texture2D)
			.DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.WrapModeS(WrapMode::Repeat).WrapModeT(WrapMode::Repeat)
			.FilterMin(Filtering::Linear).FilterMag(Filtering::Linear)
			.Build();

		m_DDGIProbeRadianceDistance = TextureResourceBuilder()
			.Name("DDGIProbeRadianceDistance").Width(m_Desc.raysPerProbe).Height(probeCounts).Dimension(TextureDim::Texture2D)
			.DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
			.WrapModeS(WrapMode::ClampToEdge).WrapModeT(WrapMode::ClampToEdge)
			.FilterMin(Filtering::Nearest).FilterMag(Filtering::Nearest)
			.Build();

		// --- Probe Ray Trace Pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					m_DDGIProbeRadianceDistance->BindTextureImage(0);

					int slot = 0;
					SetUniforms(shader, slot);

					int raysPerProbe = m_Desc.raysPerProbe;
					int probesCount = m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z;
					shader->DispatchCompute(raysPerProbe, probesCount, 1);

					shader->Unbind();

					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
				};

			m_ProbeRayTracePass = std::move(RenderPassBuilder()
				.Name("DDGIProbeRayTrace Pass")
				.Shader((shaderPathRoot / "ProbeTrace.glsl").string())
				//.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
		}

		// --- Update irradiance pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					int slot = 0;
					SetUniforms(shader, slot);

					m_DDGIProbeIrradiance->BindTextureImage(0);
					m_DDGIProbeRadianceDistance->BindTextureImage(1);

					int probesCount = m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z;
					shader->DispatchCompute(probesCount, 1, 1);
					shader->Unbind();

					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

				};

			m_ProbeUpdateIrradiancePass = std::move(RenderPassBuilder()
				.Name("DDGI Update Irradiance Pass")
				.Shader((shaderPathRoot / "UpdateIrradiance.glsl").string())
				//.Usage(ShaderUsage::PathTracing)
				.Func(Func)
				.Build());
		}

		// --- Update distance pass ---
		{
			auto Func =
				[this](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();

					int slot = 0;
					SetUniforms(shader, slot);

					m_DDGIProbeDistance->BindTextureImage(0);
					m_DDGIProbeRadianceDistance->BindTextureImage(1);
					
					int probesCount = m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z;
					shader->DispatchCompute(probesCount, 1, 1);
					shader->Unbind();

					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

				};

			m_ProbeUpdateDistancePass = std::move(RenderPassBuilder()
				.Name("DDGI Update Distance Pass")
				.Shader((shaderPathRoot / "UpdateDistance.glsl").string())
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
				.Name("DDGIProbeVisualTexture").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::RGBA).InternalFormat(InternalFormat::RGBA16F)
				.Build();
			auto probeVisualDepthTexture = TextureResourceBuilder()
				.Name("DDGIProbeDepth").Width(1280).Height(720).DataType(DataType::Float).DataFormat(DataFormat::Depth).InternalFormat(InternalFormat::Depth32F)
				.Build();
			m_ProbeVisualTex = probeVisualTexture.get();

			auto Func =
				[this](RenderPassBase& self) {
					self.GetRenderTarget()->Bind();
					RenderCommand::Clear(ClearFlags::Depth_Buffer | ClearFlags::Color_Buffer);
					auto shader = self.GetShader();
					shader->Bind();
					m_ProbeVAO->Bind();

					int totalProbes = m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z;
					for (int i = 0; i < totalProbes; i++) {
						int width = m_Desc.probeCounts.x;
						int height = m_Desc.probeCounts.y;
						int depth = m_Desc.probeCounts.z;

						int probeIndex = i;
						int x = probeIndex % width;
						int y = probeIndex / (width * depth);
						int z = (probeIndex % (width * depth)) / width;
					
						glm::vec3 pos = m_Desc.origin + glm::vec3(x, y, z) * m_Desc.probeSpacing;
						glm::mat4 model = glm::mat4(1.0);
						model = glm::translate(model, pos);
						model = glm::scale(model, glm::vec3(0.1));
						shader->SetMat4("u_Model", model);
						RenderCommand::DrawIndexed(m_ProbeVAO.get());


						break;
					}
					
					m_ProbeVAO->Unbind();
					self.GetRenderTarget()->Unbind();
					shader->Unbind();
				};

			m_ProbeVisualizationPass = std::move(RenderPassBuilder()
				.Name("DDGIProbeVisual Pass")
				.Shader((shaderPathRoot / "ProbeVisual.glsl").string())
				.AttachTarget(probeVisualTexture)
				.AttachTarget(probeVisualDepthTexture)
				.Func(Func)
				.Build());
		}

		// --- Composite Pass ---
		{
			auto litScene = g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("Lit Scene Result");
			auto sceneDepth = g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("Scene Depth");
			auto probeVisualDepthTexture = g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("DDGIProbeDepth");
			auto probeVisualTexture = g_RuntimeGlobalCtx.m_Resourcemanager->GetBufferTexture("DDGIProbeVisualTexture");
			auto Func =
				[this, litScene](RenderPassBase& self) {
					auto shader = self.GetShader();
					shader->Bind();
					uint32_t slot = 0;
					self.BindRegisteredTextureBuffers(slot);
					litScene->BindTextureImage(0);
					constexpr static int groups = 32;
					int width = litScene->GetWidth();
					int height = litScene->GetHeight();
					int numGroupsX = (width + groups - 1) / groups;
					int numGroupsY = (height + groups - 1) / groups;
					shader->DispatchCompute(numGroupsX, numGroupsY, 1);
					shader->Unbind();
				};

			m_CompositePass = std::move(RenderPassBuilder()
				.Name("DDGIComposite Pass")
				.Shader((shaderPathRoot / "Composite.glsl").string())
				//.Usage(ShaderUsage::DDGIComposite)
				.Input("u_SceneDepth", sceneDepth)
				.Input("u_DDGIProbeDepth", probeVisualDepthTexture)
				.Input("u_DDGIProbeScene", probeVisualTexture)
				.Func(Func)
				.Build());
		}
	}
	static glm::vec3 RandomUnitVector() {
		static std::mt19937 rng(std::random_device{}());
		static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		glm::vec3 v;
		do {
			v = glm::vec3(dist(rng), dist(rng), dist(rng));
		} while (glm::length2(v) > 1.0f || glm::length2(v) < 0.01f);
		return glm::normalize(v);
	}

	static glm::mat3 GenerateRandomRotationMatrix(float maxAngleRadians = 0.25f) {
		glm::vec3 axis = RandomUnitVector();
		float angle = maxAngleRadians * (float(rand()) / float(RAND_MAX)); // [0, maxAngle]

		glm::quat q = glm::angleAxis(angle, axis);
		return glm::mat3_cast(q); // 转成 mat3
	}

	void DDGIPipeline::Execute() {
		m_RandomRotation = GenerateRandomRotationMatrix();

		m_ProbeVisualizationPass->Execute();
		m_ProbeRayTracePass->Execute();
		m_ProbeUpdateIrradiancePass->Execute();
		m_ProbeUpdateDistancePass->Execute();
		m_CompositePass->Execute();
	}

	bool DDGIPipeline::Resize(uint32_t width, uint32_t height) const {
		bool resized = false;
		resized |= m_ProbeVisualTex->Resize(width, height);
		resized |= m_ProbeVisualizationPass->Resize(width, height);
		return resized;
	}

	_Texture* DDGIPipeline::GetProbeIrradianceTex() const {
		return m_DDGIProbeIrradiance.get();
	}

	_Texture* DDGIPipeline::GetProbeDistanceTex() const {
		return m_DDGIProbeDistance.get();
	}

	void DDGIPipeline::SetUniforms(Shader* shader, uint32_t slot) const {
		shader->SetFloat("u_DDGIVolumeDesc.hysteresis", m_Desc.hysteresis);
		shader->SetFloat("u_DDGIVolumeDesc.probeSpacing", m_Desc.probeSpacing);
		shader->SetInt("u_DDGIVolumeDesc.raysPerProbe", m_Desc.raysPerProbe);
		shader->SetIvec3("u_DDGIVolumeDesc.probeCounts", m_Desc.probeCounts);
		shader->SetVec3("u_DDGIVolumeDesc.origin", m_Desc.origin);
		shader->SetUint("u_DDGIVolumeDesc.probeCount", m_Desc.probeCounts.x * m_Desc.probeCounts.y * m_Desc.probeCounts.z);
		shader->SetMat3("u_DDGIVolumeDesc.randomRotation", m_RandomRotation);

		shader->SetInt("u_DDGIVolumeDesc.DDGIIrradiance", slot++);
		RenderCommand::BindTextureUnit(slot++, m_DDGIProbeIrradiance->GetTextureID());
		shader->SetInt("u_DDGIVolumeDesc.DDGIDistance", slot++);
		RenderCommand::BindTextureUnit(slot++, m_DDGIProbeDistance->GetTextureID());
	}
}