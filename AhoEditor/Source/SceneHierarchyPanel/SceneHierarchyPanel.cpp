#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include "entt.hpp"

namespace Aho {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene) {
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender() {
		return;
		ImGui::Begin("Scene Hierarchy");
		//auto [width, height] = ImGui::GetContentRegionAvail();
		auto [width, height] = ImGui::GetWindowSize();
		m_Width = width;
		m_Height = height;
		if (m_Context) {
			//for (const auto& aobject : m_Context->m_EntityManager.view<TagComponent>()) {
			//	Entity entity{ aobject , m_Context.get() };
			//	DrawEntityNode(entity);
			//}
			//if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			//	m_SelectionContext = {};
			//}
			// Right-click on blank space
			//if (ImGui::BeginPopupContextWindow(nullptr, 1)) {
			//	if (ImGui::MenuItem("Create Empty Entity"))
			//		m_Context->CreateAObject("Empty Entity");
			//	ImGui::EndPopup();
			//}
		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectionContext) {
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
		//auto& tag = entity.GetComponent<TagComponent>().Tag;

		//ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		//flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		//bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		//if (ImGui::IsItemClicked()) {
		//	m_SelectionContext = entity;
		//}

		//bool entityDeleted = false;
		//if (ImGui::BeginPopupContextItem()) {
		//	if (ImGui::MenuItem("Delete Entity"))
		//		entityDeleted = true;

		//	ImGui::EndPopup();
		//}

		//if (opened) {
		//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		//	bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
		//	if (opened)
		//		ImGui::TreePop();
		//	ImGui::TreePop();
		//}

		//if (entityDeleted) {
		//	m_Context->DestroyEntity(entity);
		//	if (m_SelectionContext == entity)
		//		m_SelectionContext = {};
		//}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity) {
	}

} // namespace Aho