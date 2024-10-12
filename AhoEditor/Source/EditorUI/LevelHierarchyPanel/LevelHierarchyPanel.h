#pragma once

#include <IamAho.h>

namespace Aho {
	class LevelHierarchyPanel {
	public:
		LevelHierarchyPanel() = default;
		LevelHierarchyPanel(const Ref<Level>& scene);
		void SetContext(const Ref<Level>& scene);
		void OnImGuiRender();
		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity);
		std::pair<float, float> GetPenalSize() { return std::make_pair(m_Width, m_Height); }
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName) {}
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		float m_Width{ 0.0f };
		float m_Height{ 0.0f };
		Ref<Level> m_Context;
		Entity m_SelectionContext;
	};
} // namespace Aho
