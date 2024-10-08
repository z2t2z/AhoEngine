#pragma once

#include "IamAho.h"
#include "SceneHierarchyPanel/SceneHierarchyPanel.h"

namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer();
		virtual ~AhoEditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		bool OnFileChanged(FileChangedEvent& event);
	private:
		bool m_CameraControlActive = false;
		float m_DeltaTime{ 0.0f };

		FileWatcher m_FileWatcher;

		std::shared_ptr<CameraManager> m_CameraManager;

		std::shared_ptr<Shader> m_Shader;
		
		std::shared_ptr<Framebuffer> m_Framebuffer;

		std::shared_ptr<Shader> m_PickingShader;
		std::shared_ptr<Framebuffer> m_PickingFBO;

		std::shared_ptr<Shader> m_SSAO_Geo;
		std::shared_ptr<Shader> m_SSAO_SSAO;
		std::shared_ptr<Shader> m_SSAO_Blur;
		std::shared_ptr<Shader> m_SSAO_Light;

		std::shared_ptr<Scene> m_ActiveScene;
		std::shared_ptr<Scene> m_EditorScene;

		AObject m_Plane; // testing game object
		AObject m_Test;

		entt::entity m_Selected{ entt::null };

		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		glm::vec2 m_ViewportSize{ 0.0f, 0.0f };
		std::unique_ptr<SceneHierarchyPanel> m_Panel;
	};

}