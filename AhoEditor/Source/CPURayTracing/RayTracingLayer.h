#pragma once

#include "IamAho.h"
#include "CPURenderer.h"

#include <chrono>

namespace Aho {
	class Timer {
	public:
		Timer() {
			reset();
		}

		void reset() {
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		float GetElapsedTime() {
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> elapsed = currentTime - m_startTime;
			return elapsed.count(); // In ms
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
	};


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
