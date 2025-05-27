#pragma once



#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class LevelLayer;
	class Entity;

	class HierachicalPanel {
	public:
		HierachicalPanel() = default;
		void Initialize(LevelLayer* m_LevelLayer);
		Entity Draw();
	private:
		LevelLayer* m_LevelLayer{ nullptr };
	};
}