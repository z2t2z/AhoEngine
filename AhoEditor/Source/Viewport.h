#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class Viewport {
	public:
		Viewport();
		void Initialize(Renderer* renderer, LevelLayer* levelLayer, ResourceLayer* resourceLayer, EventManager* manager, const std::shared_ptr<Camera>& camera);
		void Draw();
		bool IsCursorInViewport() {
			return (m_MouseX >= 0 && m_MouseY >= 0 && m_MouseX < m_ViewportWidth && m_MouseY < m_ViewportHeight);
		}
		bool IsViewportFocused() {
			return ImGui::IsWindowFocused();
		}
	private:
		void DrawToolBarOverlay();
		void DrawToolBarAddObjectBtn();
		void DrawToolBarRenderModeSelectionBtn();
		void DrawGizmo();
		void DrawManipulationToolBar();
		void DrawLightIcons();
		void TryGetDragDropTarget();
	private:
		Entity m_EnvEntity;
		ResourceLayer* m_ResourceLayer{ nullptr };
	private:
		bool m_ShouldPickObject{ false };
		uint32_t m_ViewportWidth, m_ViewportHeight;
		int m_MouseX, m_MouseY;
	private:
		std::shared_ptr<Texture2D> m_LightIcon{ nullptr };
		std::shared_ptr<Texture2D> m_CursorIcon{ nullptr };
		std::shared_ptr<Texture2D> m_AddIcon{ nullptr };
		std::shared_ptr<Texture2D> m_TranslationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_RotationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_ScaleIcon{ nullptr };
	private:
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Camera> m_EditorCamera;
		Renderer* m_Renderer{ nullptr };
		LevelLayer* m_LevelLayer{ nullptr };
	};
}