#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/PathTracing/PathTracingPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderSkyPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/DeferredShadingPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/IBLPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/PostprocessPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/DebugVisualPipeline.h"

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
		~Renderer() {
			delete m_RP_IBL;
			delete m_RP_Sky;
			delete m_RP_DeferredShading;
			delete m_RP_Postprocess;
			delete m_RP_PathTraciing;
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
		void SetupUBOs();
	private:
		IBLPipeline* m_RP_IBL; // precompute pipeline, its Excute() will be called whenever an IBL source is added
		RenderSkyPipeline* m_RP_Sky;
		DeferredShadingPipeline* m_RP_DeferredShading;
		PostprocessPipeline* m_RP_Postprocess;
		PathTracingPipeline* m_RP_PathTraciing{ nullptr };
		DebugVisualPipeline* m_RP_Dbg{ nullptr };
	private:
		float m_FrameTime{ 0.0 }; // In seconds
		bool m_CameraDirty{ false };
		RenderMode m_CurrentRenderMode{ RenderMode::DefaultLit };
	};
}

