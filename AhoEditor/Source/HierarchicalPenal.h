#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class Entity;

	class HierachicalPanel {
	public:
		HierachicalPanel() = default;
		void Initialize();
		Entity Draw();
	};
}