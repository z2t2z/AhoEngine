#include "HierarchicalPenal.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"


namespace Aho {
	bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		bool modified = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) {
			modified = true;
			values.x = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) {
			modified = true;
			values.y = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) {
			modified = true;
			values.z = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return modified;
	}

	void HierachicalPanel::Initialize(LevelLayer* levelLayer) {
		m_LevelLayer = levelLayer;
	}


	Entity HierachicalPanel::Draw() {
		Entity selectedEntity;
		static uint32_t m_PickPixelData{ UINT_MAX };

		ImGui::Begin(ICON_FA_TREE " Hierarchy");
		auto scene = m_LevelLayer->GetCurrentLevel();
		if (!scene) {
			ImGui::End();
			return selectedEntity;
		}

		auto entityManager = scene->GetEntityManager();
		auto view = scene->GetEntityManager()->GetView<TagComponent>();
		view.each([&](auto entity, TagComponent& tag) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			if (/*true || entityManager->HasComponent<PointLightComponent>(entity) || */entityManager->HasComponent<RootComponent>(entity)) {
				if (ImGui::TreeNodeEx(tag.Tag.c_str(), flags)) {
					if (ImGui::IsItemClicked()) {
						selectedEntity = entity;
						m_PickPixelData = static_cast<uint32_t>(entity);
						RendererGlobalState::g_SelectedEntityID = m_PickPixelData;
					}

					if (entityManager->HasComponent<RootComponent>(entity)) {
						for (const auto& subEntity : entityManager->GetComponent<RootComponent>(entity).entities) {
							std::string tag = (entityManager->HasComponent<TagComponent>(Entity(subEntity)) ?
								entityManager->GetComponent<TagComponent>(Entity(subEntity)).Tag : "Untitled");

							if (ImGui::TreeNodeEx(tag.c_str(), flags)) {
								if (ImGui::IsItemClicked()) {
									selectedEntity = subEntity;
									m_PickPixelData = static_cast<uint32_t>(subEntity);
									RendererGlobalState::g_SelectedEntityID = m_PickPixelData;
								}

								// If has skeleton
								if (entityManager->HasComponent<SkeletalComponent>(selectedEntity)) {
									const auto& skeletalComponent = entityManager->GetComponent<SkeletalComponent>(selectedEntity);
									//DrawNode(skeletalComponent.root);
								}
								ImGui::TreePop();
							}
						}
					}

					// If has skeleton
					if (entityManager->HasComponent<SkeletalComponent>(selectedEntity)) {
						const auto& skeletalComponent = entityManager->GetComponent<SkeletalComponent>(selectedEntity);
						//DrawNode(skeletalComponent.root);
					}

					ImGui::TreePop();
				}
			}

			});
		ImGui::End();
		return selectedEntity;
	}
}
