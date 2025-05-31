#pragma once

#include "RenderPipeline/RenderPipeline.h"
#include "RenderPipeline/PathTracing/PathTracingPipeline.h"
#include "RenderPipeline/RenderSkyPipeline.h"
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
	// TODO; Seems like a big bad way
	struct RendererGlobalState {
		static bool g_IsEntityIDValid;
		static int g_SelectedEntityID;
		static bool g_ShowDebug;
		static std::shared_ptr<RenderData> g_SelectedData;
	};
	
	class Renderer {
	public:
		Renderer();
		void Initialize();
		~Renderer() {
			AHO_CORE_INFO("~Renderer Called");
			//delete m_RP_Sky;
			//delete m_RP_DeferredShading;
			//delete m_RP_Postprocess;
			delete m_RP_PathTracing;
			delete m_RP_Derferred;
			delete m_RP_SkyAtmospheric;
			delete m_RP_IBLPipeline;
		}
		void SetRenderMode(RenderMode mode) { m_CurrentRenderMode = mode; }
		RenderMode GetRenderMode() { return m_CurrentRenderMode; }
		void Render(float deltaTime);
		RenderPipeline* GetPipeline(RenderPipelineType type);
		void SetCameraDirty();
		bool OnViewportResize(uint32_t width, uint32_t height);
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		uint32_t GetRenderResultTextureID();
		float GetFrameTime() const { return m_FrameTime; } // Return render frame time in seconds
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data);
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data);
	private:
		void SetupUBOs() const;
		void UpdateUBOs() const;
	private:
		RenderSkyPipeline* m_RP_Sky;
		DeferredShadingPipeline* m_RP_DeferredShading;
		PostprocessPipeline* m_RP_Postprocess;
		DebugVisualPipeline* m_RP_Dbg{ nullptr };
		
	// New System
	public:
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
		bool m_CameraDirty{ false };
		RenderMode m_CurrentRenderMode{ RenderMode::DefaultLit };
	};
}

