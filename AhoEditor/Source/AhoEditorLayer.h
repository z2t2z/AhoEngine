#pragma once

#include "IamAho.h"

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
		bool m_CameraControlActive = false;

	private:
		float m_DeltaTime = 0.0f;

		//AssetManagerEditor* m_Manager{ nullptr };

		std::shared_ptr<CameraManager> m_CameraManager;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_CubeVA;
		
		std::shared_ptr<Framebuffer> m_Framebuffer;

		std::shared_ptr<Scene> m_ActiveScene;
		std::shared_ptr<Scene> m_EditorScene;

		AObject m_Plane; // testing game object
		AObject m_Test;
		AObject m_CameraAObject;
		AObject m_Cube;

		uint32_t testID;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec4 m_Color; // for debug
	};

}