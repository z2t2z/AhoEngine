#include "PropertiesPanel.h"
#include "IamAho.h"
#include "HierarchicalPenal.h"
#include "EditorUI/EditorGlobalContext.h"
#include "EditorUI/ImGuiHelpers.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include "Runtime/Function/Renderer/Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Aho {
	static ImVec2 s_ImageSize(100.0f, 100.0f);
	static ImVec2 s_Padding(10.0f, 10.0f);

	PropertiesPanel::PropertiesPanel() {
	}
	
	// TODO: Needs reflction
	void PropertiesPanel::Draw() {
		const Entity& selectedEntity = g_EditorGlobalCtx.GetSelectedEntity();
		ImGui::Begin(ICON_FA_GEAR " Inspector");
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		if (!selectedEntity.Valid() || !ecs->HasComponent<GameObjectComponent>(selectedEntity)) {
			ImGui::End();
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		GameObjectComponent& goComp = ecs->GetComponent<GameObjectComponent>(selectedEntity);

		ImFont* boldFont = io.Fonts->Fonts[0];
		ImGui::PushFont(boldFont);
		ImGui::Text(goComp.name.c_str());
		ImGui::PopFont();

		DrawComponents<_TransformComponent, _MaterialComponent, LightComponent>(ecs, selectedEntity);
		ImGui::End();
		return; 
	}

	std::shared_ptr<_Texture> PropertiesPanel::TryGetDragDropTargetTexture() {
		//if (ImGui::BeginDragDropTarget()) {
		//	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_TEXTURE");
		//	if (payload) {
		//		const char* droppedData = static_cast<const char*>(payload->Data);
		//		std::string droppedString = std::string(droppedData, payload->DataSize);
		//		AHO_TRACE("Payload accepted: {}", droppedString);
		//		auto texture = Texture2D::Create(droppedString);
		//		return texture;
		//	}
		//	ImGui::EndDragDropTarget();
		//}
		return nullptr;
	}
}