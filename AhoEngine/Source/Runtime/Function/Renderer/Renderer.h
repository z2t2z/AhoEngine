#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline//PathTracing/PathTracingPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderSkyPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/DeferredShadingPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/IBLPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/PostprocessPipeline.h"
#include "Runtime/Function/Renderer/RenderPipeline/DebugVisualPipeline.h"

#include <memory>
#include <typeindex>
#include <glm/glm.hpp>

namespace Aho {

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

		void Render();
		
		RenderPipeline* GetPipeline(RenderPipelineType type) { 
			switch (type) {
				case RenderPipelineType::RPL_DeferredShading:
					return m_RP_DeferredShading;
				case RenderPipelineType::RPL_RenderSky:
					return m_RP_Sky;
				case RenderPipelineType::RPL_PostProcess:
					return m_RP_Postprocess;
				case RenderPipelineType::RPL_PathTracing:
					return m_RP_PathTraciing;
				case RenderPipelineType::RPL_IBL:
					return m_RP_IBL;
				case RenderPipelineType::RPL_DebugVisual:
					return m_RP_Dbg;
				default:
					AHO_CORE_ASSERT(false);
			}
			AHO_CORE_ASSERT(false);
		}

		void SetCameraDirty();

		bool OnViewportResize(uint32_t width, uint32_t height);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		uint32_t GetRenderResultTextureID();

		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { RenderTask::m_SceneData.push_back(data); }
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) { for (const auto& d : data) AddRenderData(d); }
	private:
		// uniform buffer objects
		void SetupUBOs();

	private:
		IBLPipeline* m_RP_IBL; // precompute pipeline, its Excute() will be called whenever an IBL source is added
		RenderSkyPipeline* m_RP_Sky;
		DeferredShadingPipeline* m_RP_DeferredShading;
		PostprocessPipeline* m_RP_Postprocess;
		PathTracingPipeline* m_RP_PathTraciing{ nullptr };
		DebugVisualPipeline* m_RP_Dbg{ nullptr };

	private:
		bool m_CameraDirty{ false };
		RenderMode m_CurrentRenderMode{ RenderMode::DefaultLit };
	};
}

