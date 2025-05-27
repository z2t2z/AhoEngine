#pragma once

#include "IamAho.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "EditorUI/ImGuiHelpers.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <memory>

namespace Aho {
	class LevelLayer;
	class Renderer;
	class _Texture;

	class PropertiesPanel {
	public:
		PropertiesPanel();
		void Initialize(LevelLayer* levelLayer, Renderer* renderer);
		void Draw(const Entity& selectedEntity);
	private:
		template<typename... Components>
		void DrawComponents(const std::shared_ptr<EntityManager>& ecs, Entity entity) {
			([&] {
				if (ecs->HasComponent<Components>(entity)) {
					auto& comp = ecs->GetComponent<Components>(entity);
					DrawComponentUI<Components>(ecs, comp);
				}
				}(), ...);
		}

		// --- Specializations for specific component types ---
		template<typename T>
		void DrawComponentUI(const std::shared_ptr<EntityManager>& ecs, T& component) {
			AHO_CORE_ASSERT(false, "DrawComponentUI not implemented for this component type.");
		}
		template<>
		void DrawComponentUI<_TransformComponent>(const std::shared_ptr<EntityManager>& ecs, _TransformComponent& transform) {
			ImGui::Separator();
			ImGuiIO& io = ImGui::GetIO();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Transform");
			ImGui::PopFont();
			bool Dirty = false;
			if (opened) {
				ImGui::Separator();
				Dirty |= DrawVec3Control("Translation", transform.Translation);
				Dirty |= DrawVec3Control("Scale", transform.Scale, 1.0f);
				Dirty |= DrawVec3Control("Rotation", transform.Rotation);
			}
		}
		template<>
		void DrawComponentUI<_MaterialComponent>(const std::shared_ptr<EntityManager>& ecs, _MaterialComponent& material) {
			ImGui::Separator();
			ImGuiIO& io = ImGui::GetIO();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			bool opened = ImGui::CollapsingHeader("Material");
			ImGui::PopFont();
			bool Dirty = false;
			MaterialDescriptor& desc = material.mat.GetMatDescriptor();
			if (opened) {
				ImGui::Separator();
				float lo = 0.0;
				float up = 1.0;
				Dirty |= ImGui::DragFloat("Metallic", &desc.metallic, 0.01f, lo, up);

				ImGui::Separator();
				Dirty |= ImGui::DragFloat("Roughness", &desc.roughness, 0.01f, lo, up);

			}
		}

		bool DrawSingleMaterialProperty(MaterialComponent& materialComp, MaterialProperty& prop);
		bool DrawMaterialProperties(MaterialComponent& materialComp);
	private:
		std::shared_ptr<_Texture> TryGetDragDropTargetTexture();
	private:
		LevelLayer* m_LevelLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
	};
}