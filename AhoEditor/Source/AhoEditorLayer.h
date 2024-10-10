#pragma once

#include "IamAho.h"
#include "SceneHierarchyPanel/SceneHierarchyPanel.h"

namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~AhoEditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnFileChanged(FileChangedEvent& event);
	private:
		bool m_CameraControlActive = false;
		float m_DeltaTime{ 0.0f };
		FileWatcher m_FileWatcher;
		Renderer* m_Renderer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		glm::vec2 m_ViewportSize{ 0.0f, 0.0f };
		std::unique_ptr<SceneHierarchyPanel> m_Panel;
	};
}