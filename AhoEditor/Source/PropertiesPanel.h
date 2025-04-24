#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class PropertiesPanel {
	public:
		PropertiesPanel();
		void Initialize(LevelLayer* levelLayer, Renderer* renderer);
		void Draw(const Entity& selectedEntity);
	private:
		bool DrawSingleMaterialProperty(MaterialComponent& materialComp, MaterialProperty& prop);
		bool DrawMaterialProperties(MaterialComponent& materialComp);
	private:
		std::shared_ptr<Texture2D> TryGetDragDropTargetTexture();
	private:
		LevelLayer* m_LevelLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
	};
}