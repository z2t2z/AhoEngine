#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
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
		void SunDirControl();
		void BVHControl();
	};
}