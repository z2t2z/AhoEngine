#include "HierarchicalPenal.h"
#include "IamAho.h"
#include "EditorUI/ImGuiHelpers.h"
#include "EditorUI/EditorGlobalContext.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"

namespace Aho {
	void HierachicalPanel::Initialize() {
	}

	void HierachicalPanel::Draw() {
		ImGui::Begin(ICON_FA_TREE " Hierarchy");
		auto DrawHierachicalTree = 
			[](auto&& self, const TreeNode& node) -> void {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				bool hasChildren = !node.children.empty();
				if (!hasChildren)
					flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (node.entity == g_EditorGlobalCtx.GetSelectedEntity())
					flags |= ImGuiTreeNodeFlags_Selected;
			
				std::string tag = std::string(ICON_FA_CUBE " ") + node.name;
				//tag += std::string("##") + std::to_string(uint32_t(node.entity.GetEntityHandle()));

				ImGui::PushID((int)node.entity.GetEntityHandle());
				bool nodeOpened = ImGui::TreeNodeEx(tag.c_str(), flags);
				ImGui::PopID();

				// Right-click context menu, TODO: Destroy logic
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
						ecs->DestroyEntity(node.entity);
						ImGui::EndPopup();
						return;
					}
					ImGui::EndPopup();
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
					g_EditorGlobalCtx.SetSelected(node.entity);
				}

				if (hasChildren && nodeOpened) {
					for (const TreeNode& child : node.children) {
						self(self, child);
					}
					ImGui::TreePop();
				}

			};

		UpdateDrawTree();
		for (const TreeNode& root : m_Roots) {
			DrawHierachicalTree(DrawHierachicalTree, root);
		}

		ImGui::End();
	}

	void HierachicalPanel::UpdateDrawTree() {
		m_Roots.clear();
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto BuildTree =
			[&ecs](auto&& self, const Entity& entity) -> TreeNode {
				const GameObjectComponent& goComp = ecs->GetComponent<GameObjectComponent>(entity);
				TreeNode node(entity, goComp.name);
				for (const Entity& child : goComp.children) {
					node.children.push_back(self(self, child));
				}
				return node;
			};

		auto view = ecs->GetView<GameObjectComponent>();
		view.each(
			[&](auto entity, GameObjectComponent& go) {
				if (go.parent.Valid()) {
					return;
				}
				m_Roots.push_back(BuildTree(BuildTree, entity));
			});
	}
}
