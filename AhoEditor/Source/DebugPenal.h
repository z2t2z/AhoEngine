#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	// For quick debug info or control
	class DebugPenal {
	public:
		DebugPenal();
		void Draw();
	private:
		void BVHControl();
		void GetSSBOData();
	};
}