#pragma once

#include <string>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class LevelLayer;
	class Entity;

	bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	class HierachicalPanel {
	public:
		HierachicalPanel() = default;
		void Initialize(LevelLayer* m_LevelLayer);
		Entity Draw();

	private:
		LevelLayer* m_LevelLayer{ nullptr };
	};
}