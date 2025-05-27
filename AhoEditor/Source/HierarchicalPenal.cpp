#include "HierarchicalPenal.h"
#include "IamAho.h"
#include "EditorUI/ImGuiHelpers.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"

namespace Aho {
	void HierachicalPanel::Initialize(LevelLayer* levelLayer) {
		m_LevelLayer = levelLayer;
	}

	Entity HierachicalPanel::Draw() {
		static Entity selectedEntity;
		ImGui::Begin(ICON_FA_TREE " Hierarchy");
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		static auto DrawHierachicalTree = 
			[&ecs](auto&& self, const Entity& entity) -> void {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				auto& goComp = ecs->GetComponent<GameObjectComponent>(entity);
				std::string tag = goComp.name;
				tag = ICON_FA_CUBE + std::string(1, ' ') + tag;
				tag += "##" + std::to_string(uint32_t(entity.GetEntityHandle()));
				if (ImGui::TreeNodeEx(tag.c_str(), flags)) {
					if (ImGui::IsItemClicked()) {
						selectedEntity = entity;
					}
					for (const Entity& child : goComp.children) {
						self(self, child);
					}
					ImGui::TreePop();
				}
			};

		auto view = ecs->GetView<GameObjectComponent>();
		view.each(
			[&](auto entity, GameObjectComponent& go) {
				if (go.parent.Valid()) {
					// Skip child entities
					return;
				}
				DrawHierachicalTree(DrawHierachicalTree, entity);
			});
		ImGui::End();
		return selectedEntity;
	}
}
