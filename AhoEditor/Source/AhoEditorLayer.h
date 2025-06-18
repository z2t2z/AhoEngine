#pragma once

#include "ContentBrowser.h"
#include "HierarchicalPenal.h"
#include "PropertiesPanel.h"
#include "Viewport.h"
#include "DebugPenal.h"

namespace Aho {
	class Entity;
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer(LevelLayer* levellayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~AhoEditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		ContentBrowser m_ContentBrowser;
		PropertiesPanel m_PropertiesPanel;
		HierachicalPanel m_HierachicalPanel;
		Viewport m_Viewport;
		DebugPenal m_DbgPenal;
		Entity m_EditorCamEntity;
	private:
		bool m_CursorLocked{ false };
		float m_DeltaTime{ 0.0f };
	private:
		LevelLayer* m_LevelLayer{ nullptr }; 
		Renderer* m_Renderer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
}