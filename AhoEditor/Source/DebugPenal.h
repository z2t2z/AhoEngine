#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class RenderSkyPipeline;
	class DeferredShadingPipeline;
	class LevelLayer;
	class PathTracingPipeline;

	// For quick debug info or control
	class DebugPenal {
	public:
		DebugPenal();
		void Draw();
	public:
		RenderSkyPipeline* m_SkyPipeline{ nullptr };
		DeferredShadingPipeline* m_ShadingPipeline{ nullptr };
		LevelLayer* m_LevelLayer{ nullptr };
		PathTracingPipeline* m_PtPipeline{ nullptr };
	private:
		bool SunDirControl();
		void BVHControl();
		void GetSSBOData();
	};
}