#pragma once

#include "ContentBrowser.h"
#include "HierarchicalPenal.h"
#include "PropertiesPanel.h"
#include "Viewport.h"

namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer(LevelLayer* levellayer, ResourceLayer* resourceLayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~AhoEditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		void DrawCircle(const ImVec2& center, float radius);
	private:
		ContentBrowser m_ContentBrowser;
		PropertiesPanel m_PropertiesPanel;
		HierachicalPanel m_HierachicalPanel;
		Viewport m_Viewport;
	private:
		bool m_PickingPass{ false };
		bool m_CursorLocked{ false };
		bool m_ShouldPickObject{ false };
		bool m_IsClickingEventBlocked{ false };
		float m_DeltaTime{ 0.0f };
	private:
		LevelLayer* m_LevelLayer{ nullptr }; 
		ResourceLayer* m_ResourceLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	private:
		Ray m_Ray;
	private:
		void TempSunDirControl(); // temporary
		void TempBVHControl();
	};
}