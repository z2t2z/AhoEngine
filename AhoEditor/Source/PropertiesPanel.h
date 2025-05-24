#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <memory>

namespace Aho {
	class LevelLayer;
	class Renderer;
	class Entity;
	class _Texture;
	struct MaterialComponent;
	struct MaterialProperty;

	class PropertiesPanel {
	public:
		PropertiesPanel();
		void Initialize(LevelLayer* levelLayer, Renderer* renderer);
		void Draw(const Entity& selectedEntity);
	private:
		bool DrawSingleMaterialProperty(MaterialComponent& materialComp, MaterialProperty& prop);
		bool DrawMaterialProperties(MaterialComponent& materialComp);
	private:
		std::shared_ptr<_Texture> TryGetDragDropTargetTexture();
	private:
		LevelLayer* m_LevelLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
	};
}