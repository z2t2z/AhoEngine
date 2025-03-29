#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		void Initialize();
		void Draw();
	private:

	};
}