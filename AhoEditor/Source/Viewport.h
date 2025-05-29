#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class _Texture;
	class Viewport {
	public:
		Viewport();
		void Initialize(Renderer* renderer, LevelLayer* levelLayer, EventManager* manager, const std::shared_ptr<Camera>& camera);
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
	private:
		void TryGetDragDropTarget();
	private:
		Entity m_EnvEntity;
	private:
		bool m_ShouldPickObject{ false };
		uint32_t m_ViewportWidth, m_ViewportHeight;
		int m_MouseX, m_MouseY;
	private:
		_Texture* m_LightIcon{ nullptr };
		_Texture* m_AddIcon{ nullptr };
	private:
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Camera> m_EditorCamera;
		Renderer* m_Renderer{ nullptr };
		LevelLayer* m_LevelLayer{ nullptr };
	};
}