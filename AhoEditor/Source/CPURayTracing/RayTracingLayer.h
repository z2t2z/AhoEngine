#pragma once

#include "IamAho.h"
#include "CPURenderer.h"

#include <chrono>

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

		void Render();
	private:
		bool OnMouseRightButtonPressed(MouseButtonPressedEvent& e) {}
		bool OnMouseRightButtonReleased(MouseButtonReleasedEvent& e) {}
		bool OnKeyPressed(KeyPressedEvent& e) {}

	private:
		CPURenderer m_Renderer;
		Timer m_Timer;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		CPUScene m_Scene;
	};


}
