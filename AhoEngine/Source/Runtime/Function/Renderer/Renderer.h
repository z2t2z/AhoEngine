#pragma once

#include "RenderPipeline/RenderPipeline.h"
#include "RenderPipeline/PathTracing/PathTracingPipeline.h"
#include "RenderPipeline/DeferredShadingPipeline.h"
#include "RenderPipeline/IBLPipeline.h"
#include "RenderPipeline/PostprocessPipeline.h"
#include "RenderPipeline/DebugVisualPipeline.h"

#include "RenderPipeline/DeferredPipeline.h"
#include "RenderPipeline/SkyAtmosphericPipeline.h"

#include <memory>
#include <typeindex>
#include <glm/glm.hpp>

namespace Aho {
	class RenderData;
	struct RenderTask;

	enum RenderMode {
		DefaultLit,
		Unlit,
		PathTracing,
		ModeCount
	};

	
	class Renderer {
	public:
		Renderer();
		void Initialize();
		~Renderer() {
			delete m_RP_PathTracing;
			delete m_RP_Derferred;
			delete m_RP_SkyAtmospheric;
			delete m_RP_IBLPipeline;
		}
		void SetRenderMode(RenderMode mode) { 
			m_CurrentRenderMode = mode; 
			if (mode == RenderMode::PathTracing) {
				m_ActivePipelines = { m_RP_PathTracing };
			} else {
				m_ActivePipelines = { m_RP_IBLPipeline, m_RP_SkyAtmospheric, m_RP_Derferred };
			}
		}
		RenderMode GetRenderMode() const { return m_CurrentRenderMode; }
		void Render(float deltaTime);
		RenderPipeline* GetPipeline(RenderPipelineType type);
		bool OnViewportResize(uint32_t width, uint32_t height);
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		uint32_t GetRenderResultTextureID();
		std::vector<RenderPassBase*> GetAllRenderPasses() const { return m_AllRenderPasses; }
		void RegisterRenderPassBase(RenderPassBase* renderPassBase) { m_AllRenderPasses.push_back(renderPassBase); }
	private:
		void SetupUBOs() const;
		void UpdateUBOs() const;
	private:
		DeferredShadingPipeline* m_RP_DeferredShading{ nullptr };
		PostprocessPipeline* m_RP_Postprocess{ nullptr };
		DebugVisualPipeline* m_RP_Dbg{ nullptr };
	private:
		std::vector<RenderPipeline*> m_ActivePipelines;
	// New System
	public:
		std::vector<RenderPassBase*> m_AllRenderPasses;
		DeferredShading* GetDeferredShadingPipeline() { return m_RP_Derferred; }
		SkyAtmosphericPipeline* GetSkyAtmosphericPipeline() { return m_RP_SkyAtmospheric; }
		PathTracingPipeline* GetPathTracingPipeline() { return m_RP_PathTracing; }
	private:
		_IBLPipeline* m_RP_IBLPipeline{ nullptr };
		DeferredShading* m_RP_Derferred{ nullptr };
		SkyAtmosphericPipeline* m_RP_SkyAtmospheric{ nullptr };
		PathTracingPipeline* m_RP_PathTracing{ nullptr };
	private:
		float m_FrameTime{ 0.0 }; // In seconds
		RenderMode m_CurrentRenderMode{ RenderMode::DefaultLit };
	};
}

