#include "Ahopch.h"
#include "Application.h"

#include "Runtime/Core/Input/Input.h"
#include <GLFW/glfw3.h>

namespace Aho {
// std::bind(): Extract a member function from a class as a new function that can be directly called, while binding some parameters in advance
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		AHO_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);
		m_ImGuiLayer = new ImGuiLayer();
		m_EventManager = new EventManager();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::Run() {
		while (m_Running) {
			float currTime = (float)glfwGetTime();
			float deltaTime = currTime - m_LastFrameTime;
			for (auto layer : m_LayerStack) {
				layer->OnUpdate(deltaTime);
			}
			m_ImGuiLayer->Begin();
			for (auto layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			while (!m_EventManager->Empty()) {
				OnEvent(*m_EventManager->PopFront());
			}
			m_Window->OnUpdate();
			// Calculate FPS
			m_AccumulatedTime += deltaTime;
			m_FPS += 1;
			if (m_AccumulatedTime >= 1.0f) {
				AHO_CORE_TRACE("{}", m_FPS);
				m_FPS = 0;
				m_AccumulatedTime = 0.0f;
			}
			m_LastFrameTime = currTime;
		}
	}

	void Application::ShutDown() {
		m_Running = false;
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			(*it)->OnEvent(e);
			if (e.Handled() && e.GetEventType() == EventType::AddEntity) {
				AHO_CORE_ERROR("{}", (*it)->GetDebugName());
				break;
			}
		}
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay) {
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
} // namespace Aho

