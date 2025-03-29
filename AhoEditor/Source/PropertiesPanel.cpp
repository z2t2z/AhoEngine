#include "PropertiesPanel.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include "HierarchicalPenal.h"

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
	
	void PropertiesPanel::Draw(Entity selectedEntity) {
		ImGui::Begin(ICON_FA_GEAR " Properties Panel");
		if (!selectedEntity.Valid()) {
			ImGui::End();
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		auto& Tagc = entityManager->GetComponent<TagComponent>(selectedEntity);

		ImFont* boldFont = io.Fonts->Fonts[0];
		ImGui::PushFont(boldFont);
		ImGui::Text(Tagc.Tag.c_str());
		ImGui::PopFont();

		ImGui::Separator();
		ImGui::PushFont(io.Fonts->Fonts[1]);
		bool opened = ImGui::CollapsingHeader("Transform");
		ImGui::PopFont();
		if (opened) {
			ImGui::Separator();
			auto& tc = entityManager->GetComponent<TransformComponent>(selectedEntity);
			auto& translation = tc.GetTranslation();
			auto& scale = tc.GetScale();
			auto& rotation = tc.GetRotation();
			tc.dirty |= DrawVec3Control("Translation", translation);
			tc.dirty |= DrawVec3Control("Scale", scale, 1.0f);
			tc.dirty |= DrawVec3Control("Rotation", rotation);
		}

		ImGui::Separator();
		ImGui::PushFont(io.Fonts->Fonts[1]);
		opened = ImGui::CollapsingHeader("Material");
		ImGui::PopFont();

		bool textureChanged = false;
		// NEED BIG REFACTOR!
		if (opened) {
			if (entityManager->HasComponent<MaterialComponent>(selectedEntity)) {
				auto& materialComp = entityManager->GetComponent<MaterialComponent>(selectedEntity);
				if (ImGui::BeginTable("TwoColumnTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
					for (auto& prop : *materialComp.material) {
						switch (prop.m_Type) {
						case TexType::Albedo:
							ImGui::TableNextColumn();
							ImGui::Text("Aledo");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, glm::vec3>) {
									ImGui::ColorPicker3("Color Picker", glm::value_ptr(value));
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Albedo };
								}
								}, prop.m_Value);
							break;
						case TexType::Normal:
							ImGui::TableNextColumn();
							ImGui::Text("Normal");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::Text("Empty");
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Normal };

									*materialComp.matMask |= MaterialMaskEnum::NormalMap;
									materialComp.handle->SetHandles(texture->GetTextureHandle(), TexType::Normal);

									textureChanged = true;
								}

								}, prop.m_Value);
							break;
						case TexType::Roughness:
							ImGui::TableNextColumn();
							ImGui::Text("Roughness");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##Roughness", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Roughness };
								}
								}, prop.m_Value);
							break;
						case TexType::Metallic:
							ImGui::TableNextColumn();
							ImGui::Text("Metallic");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##Metallic", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Metallic };
								}
								}, prop.m_Value);
							break;
						case TexType::AO:
							ImGui::TableNextColumn();
							ImGui::Text("AO");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##AO", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::AO };
								}
								}, prop.m_Value);
							break;
						default:
							continue;
						}
					}
					ImGui::EndTable();
				}
				if (ImGui::Button(ICON_FA_FILE_CIRCLE_PLUS " New Slot")) {
					ImGui::OpenPopup("popup_menu_material");
				}
				ImVec2 buttonMin = ImGui::GetItemRectMin(); // upper left
				ImVec2 buttonMax = ImGui::GetItemRectMax(); // lower right
				ImVec2 nxtPos = ImVec2(buttonMin.x, buttonMax.y);
				ImGui::SetNextWindowPos(nxtPos, ImGuiCond_Always);
				if (ImGui::BeginPopup("popup_menu_material")) {
					if (!materialComp.material->HasProperty(TexType::Normal)) {
						if (ImGui::MenuItem("Normal")) {
							materialComp.material->AddMaterialProperties({ 0.0f, TexType::Normal });
						}
					}
					if (!materialComp.material->HasProperty(TexType::Metallic)) {
						if (ImGui::MenuItem("Metallic")) {
							materialComp.material->AddMaterialProperties({ 0.0f, TexType::Metallic });
						}
					}
					if (!materialComp.material->HasProperty(TexType::Roughness)) {
						if (ImGui::MenuItem("Roughness")) {
							materialComp.material->AddMaterialProperties({ 1.0f, TexType::Roughness });
						}
					}
					if (!materialComp.material->HasProperty(TexType::AO)) {
						if (ImGui::MenuItem("AO")) {
							materialComp.material->AddMaterialProperties({ 0.2f, TexType::AO });
						}
					}
					ImGui::EndPopup();
				}
			}

			if (textureChanged) {
				m_LevelLayer->UpdatePathTracingTextureHandlesSSBO();
				//m_LevelLayer->GetCurrentLevel()->GetTLAS().
				auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
				auto view = entityManager->GetView<BVHComponent, TransformComponent>();

				auto& materialComp = entityManager->GetComponent<MaterialComponent>(selectedEntity);
				MaterialMaskEnum mask = *materialComp.matMask;
				view.each(
					[&mask](auto entity, BVHComponent& bc, TransformComponent& tc) {
						for (BVHi* bvh : bc.bvhs) {
							bvh->UpdateMaterialMask(mask);
							bvh->ApplyTransform(tc.GetTransform());
						}
					});

				BVHi& alts = m_LevelLayer->GetCurrentLevel()->GetTLAS();
				alts.UpdateTLAS();

				PathTracingPipeline* ptpl = static_cast<PathTracingPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_PathTracing));
				ptpl->UpdateSSBO(m_LevelLayer->GetCurrentLevel());

			}
		}

		if (entityManager->HasComponent<PointLightComponent>(selectedEntity)) {
			auto& pc = entityManager->GetComponent<PointLightComponent>(selectedEntity);
			ImGui::Separator();
			ImGui::Text("Light Color:");
			ImGui::ColorPicker3("Color Picker", glm::value_ptr(pc.color));
		}
		ImGui::End();
	}

	std::shared_ptr<Texture2D> PropertiesPanel::TryGetDragDropTargetTexture() {
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_TEXTURE");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				std::string droppedString = std::string(droppedData, payload->DataSize);
				AHO_TRACE("Payload accepted! {}", droppedString);
				auto texture = Texture2D::Create(droppedString);
				return texture;
			}
			ImGui::EndDragDropTarget();
		}
		return nullptr;
	}
}