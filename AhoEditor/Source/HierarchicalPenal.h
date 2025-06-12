#pragma once

#include "Runtime/Function/Level/EcS/Entity.h"
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class HierachicalPanel {
	public:
		HierachicalPanel() = default;
		void Initialize();
		void Draw();
	private:
		void UpdateDrawTree();
	private:
		struct TreeNode {
			TreeNode(const Entity& entity, const std::string& name) 
				: entity(entity), name(name) {}
			Entity entity;
			std::string name;
			std::vector<TreeNode> children;
		};
		std::vector<TreeNode> m_Roots;
	};
}