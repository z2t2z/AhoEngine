#include "PropertiesPanel.h"
#include "IamAho.h"
#include "HierarchicalPenal.h"
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

	void PropertiesPanel::Initialize(LevelLayer* levelLayer, Renderer* renderer) {
		m_LevelLayer = levelLayer;
		m_Renderer = renderer;
	}
	
	// TODO: Need reflction
	void PropertiesPanel::Draw(const Entity& selectedEntity) {
		ImGui::Begin(ICON_FA_GEAR " Properties Panel");
		if (!selectedEntity.Valid()) {
			ImGui::End();
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		GameObjectComponent& goComp = ecs->GetComponent<GameObjectComponent>(selectedEntity);

		ImFont* boldFont = io.Fonts->Fonts[0];
		ImGui::PushFont(boldFont);
		ImGui::Text(goComp.name.c_str());
		ImGui::PopFont();

		DrawComponents<_TransformComponent, _MaterialComponent>(ecs, selectedEntity);
		ImGui::End();
		return; 

		// Delete the following codes

		if (ecs->HasComponent<PointLightComponent>(selectedEntity)) {
			auto& pc = ecs->GetComponent<PointLightComponent>(selectedEntity);
			ImGui::Separator();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Light Settings");
			ImGui::PopFont();
			if (opened) {
				ImGui::Text("Light Color:");
				ImGui::ColorPicker3("Color Picker", glm::value_ptr(pc.color));
			}
		}

		if (ecs->HasComponent<SkyComponent>(selectedEntity)) {
			auto& sky = ecs->GetComponent<SkyComponent>(selectedEntity);

			bool changed = ImGui::DragFloat3("SunRotation", &sky.Direction[0], 0.1f);
			float theta = glm::radians(sky.Direction.x), phi = glm::radians(sky.Direction.y);
			sky.DirectionXYZ = normalize(glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi)));

			ImGui::DragFloat("Intensity", &sky.Intensity, 0.1f);
		}


		if (ecs->HasComponent<EnvComponent>(selectedEntity)) {
			auto& pc = ecs->GetComponent<EnvComponent>(selectedEntity);
			ImGui::Separator();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Enviornment Textures(HDR)");
			ImGui::PopFont();
			if (opened) {
				for (const Texture* texture : pc.envTextures) {
					ImGui::Image((ImTextureID)texture->GetTextureID(), s_ImageSize);
					
					//auto id = static_cast<PathTracingPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_PathTracing))->GetIBL()->Get2DCDF();
					//ImGui::Image((ImTextureID)id, s_ImageSize);

				}
			}
		}
		ImGui::End();
	}


	bool PropertiesPanel::DrawSingleMaterialProperty(MaterialComponent& materialComp, MaterialProperty& prop) {
		return false;
		//bool textureChanged = false;
		//ImGui::TableNextColumn();
		//ImGui::Text(MaterialDescriptor::s_Umap.at(prop.m_Type).c_str());
		//ImGui::TableNextColumn();
		//MaterialDescriptor& handle = m_LevelLayer->GetMaterialDescriptor(materialComp.meshId);
		//std::visit(
		//	[&](auto& value) {
		//		bool valueChanged = false;
		//		using T = std::decay_t<decltype(value)>;
		//		if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
		//			ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
		//		}
		//		else if constexpr (std::is_same_v<T, glm::vec3>) {
		//			if (prop.m_Type != TexType::Normal) {
		//				std::string displayName = "ColorPicker";
		//				displayName += (prop.m_Type == TexType::Albedo ? "baseColor" : "emissive");
		//				valueChanged |= ImGui::ColorPicker3(displayName.c_str(), glm::value_ptr(value));
		//			}
		//			else {
		//				ImGui::Text("Empty");
		//			}
		//		}
		//		else if constexpr (std::is_same_v<T, float>) {
		//			std::string displayname = "##" + MaterialDescriptor::s_Umap.at(prop.m_Type);
		//			float lo = prop.m_Type == TexType::ior ? 1.001 : 0.0;
		//			float up = prop.m_Type == TexType::ior ? 2.5 : 1.0;
		//			lo = prop.m_Type == TexType::EmissiveScale ? 0.0f : lo;
		//			up = prop.m_Type == TexType::EmissiveScale ? 1000.0f : up;
		//			valueChanged |= ImGui::DragFloat(displayname.c_str(), &value, 0.01f, lo, up);
		//		}
		//		auto texture = TryGetDragDropTargetTexture();
		//		if (texture) {
		//			handle.SetHandles(texture->GetTextureHandle(), prop.m_Type);
		//			prop = { texture, prop.m_Type };
		//			textureChanged = true;
		//		}
		//		if (!texture && valueChanged) {
		//			textureChanged = true;
		//			handle.SetValue(value, prop.m_Type);
		//		}
		//	}, prop.m_Value);
		//return textureChanged;
	}

	bool PropertiesPanel::DrawMaterialProperties(MaterialComponent& materialComp) {
		bool textureChanged = false;
		if (ImGui::BeginTable("TwoColumnTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
			for (auto& prop : *materialComp.material) {
				textureChanged |= DrawSingleMaterialProperty(materialComp, prop);
			}
			ImGui::EndTable();
		}

		return textureChanged;
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