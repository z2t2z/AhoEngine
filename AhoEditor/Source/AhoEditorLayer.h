#pragma once

#include "IamAho.h"


namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer();
		virtual ~AhoEditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate() override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		//void NewScene();
		//void OpenScene();
		//void SaveSceneAs();

	private:
		//Hazel::OrthographicCameraController m_CameraController;
		// Temp
		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_CubeVA;
		
		std::shared_ptr<Framebuffer> m_Framebuffer;

		std::shared_ptr<Scene> m_ActiveScene;
		std::shared_ptr<Scene> m_EditorScene;

		Entity m_Test;
		Entity m_CameraEntity;
		Entity m_Cube;
		Camera* m_Camera{ nullptr };
		glm::vec4 m_Color; // for debug
		//Ref<Scene> m_ActiveScene;
		//Entity m_SquareEntity;
		//Entity m_CameraEntity;
		//Entity m_SecondCamera;

		//bool m_PrimaryCamera = true;

		//EditorCamera m_EditorCamera;

		//Ref<Texture2D> m_CheckerboardTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		int m_GizmoType = -1;

		// Panels
		//SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}