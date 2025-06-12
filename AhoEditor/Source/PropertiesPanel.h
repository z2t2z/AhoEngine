#pragma once

#include "IamAho.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "EditorUI/ImGuiHelpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>
#include <memory>

namespace Aho {
	class LevelLayer;
	class Renderer;
	class _Texture;

	class PropertiesPanel {
	public:
		PropertiesPanel();
		void Draw();
	private:
		template<typename... Components>
		void DrawComponents(const std::shared_ptr<EntityManager>& ecs, Entity entity) {
			([&] {
				if (ecs->HasComponent<Components>(entity)) {
					auto& comp = ecs->GetComponent<Components>(entity);
					DrawComponentUI<Components>(ecs, entity, comp);
				}
				}(), ...);
		}

		// --- Specializations for specific component types ---
		template<typename T>
		void DrawComponentUI(const std::shared_ptr<EntityManager>& ecs, Entity entity, T& component) {
			AHO_CORE_ASSERT(false, "DrawComponentUI not implemented for this component type.");
		}
		template<typename T>
		void DrawComponentUI(const std::shared_ptr<EntityManager>& ecs, Entity entity, LightComponent& lightComp) {
			auto light = lightComp.light;
			bool changed = false;
			if (ImGuiHelpers::DrawVec3Control("Color", light->GetColor())) changed = true;
			ImGui::Separator();
			if (ImGui::DragFloat("Intensity", &light->GetIntensity(), 0.01f, 0.0f, 1000.0f)) changed = true;
			switch (light->GetType()) {
				case LightType::Directional: {
					std::shared_ptr<DirectionalLight> dirLight = std::static_pointer_cast<DirectionalLight>(light);
					glm::vec3 dirEuler = Math::DirectionToEuler(dirLight->GetDirection());
					if (ImGuiHelpers::DrawVec3Control("Direction", dirEuler)) {
						dirLight->SetDirection(glm::normalize(Math::EulerToDirection(dirEuler)));
						changed = true;
					}
					break;
				}
				case LightType::Point:
					AHO_CORE_ASSERT(false, "Point light properties not implemented yet.");
					break;
				case LightType::Spot:
					AHO_CORE_ASSERT(false, "Spot light properties not implemented yet.");
					break;
				default:
					break;
			}
			if (changed && !ecs->HasComponent<LightDirtyTagComponent>(entity)) {
				ecs->AddComponent<LightDirtyTagComponent>(entity);
			}
		}
		template<>
		void DrawComponentUI<_TransformComponent>(const std::shared_ptr<EntityManager>& ecs, Entity entity, _TransformComponent& transform) {
			ImGui::Separator();
			ImGuiIO& io = ImGui::GetIO();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Transform");
			ImGui::PopFont();
			transform.Dirty = false;
			if (opened) {
				ImGui::Separator();
				transform.Dirty |= ImGuiHelpers::DrawVec3Control("Translation", transform.Translation);
				transform.Dirty |= ImGuiHelpers::DrawVec3Control("Scale", transform.Scale, 1.0f);
				transform.Dirty |= ImGuiHelpers::DrawVec3Control("Rotation", transform.Rotation);
			}
		}
		template<>
		void DrawComponentUI<_MaterialComponent>(const std::shared_ptr<EntityManager>& ecs, Entity entity, _MaterialComponent& material) {
			ImGui::Separator();
			ImGuiIO& io = ImGui::GetIO();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Material");
			ImGui::PopFont();
			material.Dirty = false;
			MaterialDescriptor& matDesc = material.mat.GetMatDescriptor();
			MaterialTextureDescriptor& texDesc = material.mat.GetMatTextureDescriptor();

			// --- Helpers ---
			auto TextureSlot = [](const char* label, _Texture*& tex, bool& useTex) {
				ImGui::PushID(label);
				ImGui::Checkbox("Use", &useTex);
				ImGui::SameLine();
				ImGui::Text("%s", label);

				ImGui::BeginDisabled(!useTex);
				ImGui::Indent(20.0f);

				ImVec2 cursorPos = ImGui::GetCursorScreenPos();
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				static ImVec2 size(64, 64); // Texture slot size
				if (tex) {
					ImGuiHelpers::RoundedImage((ImTextureID)(tex->GetTextureID()), size, 4.0f);
				}
				else {
					// Draw gray box
					ImU32 bgColor = IM_COL32(80, 80, 80, 255);
					drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + size.x, cursorPos.y + size.y), bgColor, 4.0f);
					drawList->AddRect(cursorPos, ImVec2(cursorPos.x + size.x, cursorPos.y + size.y), IM_COL32_WHITE, 4.0f);

					// Centered "Empty" text
					const char* emptyText = "Empty";
					ImVec2 textSize = ImGui::CalcTextSize(emptyText);
					ImVec2 textPos = ImVec2(cursorPos.x + (size.x - textSize.x) * 0.5f, cursorPos.y + (size.y - textSize.y) * 0.5f);
					drawList->AddText(textPos, IM_COL32_WHITE, emptyText);

					ImGui::Dummy(size);  // Reserve space
				}
				ImGui::Unindent(20.0f);
				ImGui::EndDisabled();
				ImGui::PopID();
			};

			auto DragScalar = [](const char* label, float& value, bool disabled = false, float step = 0.01f, float min = 0.0f, float max = 1.0f) {
				ImGui::BeginDisabled(disabled);
				bool c = ImGui::DragFloat(label, &value, step, min, max);
				ImGui::EndDisabled();
				return c;
			};

			auto DragVec3 = [](const char* label, glm::vec3& value, bool disabled = false, float step = 0.01f, float min = 0.0f, float max = 1.0f) {
				ImGui::BeginDisabled(disabled);
				bool c = ImGui::ColorEdit3(label, glm::value_ptr(value),
					ImGuiColorEditFlags_DisplayRGB |
					ImGuiColorEditFlags_InputRGB |
					ImGuiColorEditFlags_NoLabel |
					ImGuiColorEditFlags_NoPicker);  // optional, if you want only click popup
				ImGui::SameLine(); ImGui::Text("%s", label); // label on the right
				ImGui::EndDisabled();
				return c;
			};

			if (opened) {
				ImGui::Separator();

				// --- Base Color ---
				TextureSlot("Base Color", texDesc.baseColorTex, texDesc.useBaseColorTex);
				material.Dirty |= DragVec3("Base Color", matDesc.baseColor, texDesc.useBaseColorTex);
				ImGui::Separator();

				// --- Normal Map ---
				TextureSlot("Normal Map", texDesc.normalMap, texDesc.useNormalMap);
				// Usually no scalar fallback
				ImGui::Separator();

				// --- Metallic ---
				TextureSlot("Metallic", texDesc.metallicTex, texDesc.useMetallicTex);
				material.Dirty |= DragScalar("Metallic", matDesc.metallic, texDesc.useMetallicTex);
				ImGui::Separator();

				// --- Roughness ---
				TextureSlot("Roughness", texDesc.roughnessTex, texDesc.useRoughnessTex);
				material.Dirty |= DragScalar("Roughness", matDesc.roughness, texDesc.useRoughnessTex);
				ImGui::Separator();

				// --- AO ---
				TextureSlot("AO", texDesc.aoTex, texDesc.useAoTex);
				material.Dirty |= DragScalar("Ambient Occlusion", matDesc.ao, texDesc.useAoTex);
				ImGui::Separator();

				// --- Emissive ---
				TextureSlot("Emissive", texDesc.emissiveTex, texDesc.useEmissiveTex);
				material.Dirty |= DragVec3("Emissive Color", matDesc.emissive, texDesc.useEmissiveTex);
				material.Dirty |= DragScalar("Emissive Scale", matDesc.emissiveScale, texDesc.useEmissiveTex);
				ImGui::Separator();

				// --- Other Scalar Parameters ---
				ImGui::SeparatorText("Advanced Parameters");
				//material.Dirty |= DragScalar("Specular", matDesc.specular);
				material.Dirty |= DragScalar("Spec Tint", matDesc.specTint);
				material.Dirty |= DragScalar("Subsurface", matDesc.subsurface);
				material.Dirty |= DragScalar("Anisotropic", matDesc.anisotropic);
				material.Dirty |= DragScalar("Sheen", matDesc.sheen);
				material.Dirty |= DragScalar("Sheen Tint", matDesc.sheenTint);
				material.Dirty |= DragScalar("Clearcoat", matDesc.clearcoat);
				material.Dirty |= DragScalar("Clearcoat Gloss", matDesc.clearcoatGloss);
				material.Dirty |= DragScalar("Spec Trans", matDesc.specTrans);
				material.Dirty |= DragScalar("IOR", matDesc.ior, false, 0.01f, 1.0f, 2.5f);
			}
		}
	private:
		std::shared_ptr<_Texture> TryGetDragDropTargetTexture();
	private:
		LevelLayer* m_LevelLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
	};
}