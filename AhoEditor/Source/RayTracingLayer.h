#pragma once

#include "IamAho.h"
#include "CPURenderer.h"

namespace Aho {
	class RayTracingLayer : public Layer {
	public:
		RayTracingLayer();
		virtual ~RayTracingLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override {}

		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override {}
	private:
		bool m_CameraControlActive = false;
		bool OnMouseRightButtonPressed(MouseButtonPressedEvent& e) {}
		bool OnMouseRightButtonReleased(MouseButtonReleasedEvent& e) {}
		bool OnKeyPressed(KeyPressedEvent& e) {}

	private:
		CPURenderer m_Renderer;

		float m_LastRenderTime = 0.0f;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		std::unique_ptr<EditorCamera> m_Camera;
		std::shared_ptr<Scene> m_ActiveScene;

	};


}
