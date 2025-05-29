#include "HierarchicalPenal.h"
#include "IamAho.h"
#include "EditorUI/ImGuiHelpers.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"

namespace Aho {
	void HierachicalPanel::Initialize() {
	}

	Entity HierachicalPanel::Draw() {
		static Entity selectedEntity;
		ImGui::Begin(ICON_FA_TREE " Hierarchy");
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto DrawHierachicalTree = 
			[&ecs](auto&& self, const Entity& entity) -> void {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

				if (entity == selectedEntity) {
					flags |= ImGuiTreeNodeFlags_Selected;
				}

				auto& goComp = ecs->GetComponent<GameObjectComponent>(entity);
				std::string tag = goComp.name;
				tag = ICON_FA_CUBE + std::string(1, ' ') + tag;
				tag += "##" + std::to_string(uint32_t(entity.GetEntityHandle()));

				bool nodeOpened = ImGui::TreeNodeEx(tag.c_str(), flags);
				if (ImGui::IsItemClicked()) {
					selectedEntity = entity;
				}

				// Right-click context menu
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						if (entity == selectedEntity)
							selectedEntity.SetInvalid();  // reset selection if deleted
						ecs->DestroyEntity(entity);
					}
					ImGui::EndPopup();
				}

				// Dfs its children
				if (nodeOpened) {
					if (entity.Valid()) {
						for (const Entity& child : goComp.children) {
							self(self, child);
						}
					}
					ImGui::TreePop();
				}
			};

		auto view = ecs->GetView<GameObjectComponent>();
		view.each(
			[&](auto entity, GameObjectComponent& go) {
				if (go.parent.Valid()) {
					return;
				}
				DrawHierachicalTree(DrawHierachicalTree, entity);
			});
		ImGui::End();
		return selectedEntity;
	}
}
