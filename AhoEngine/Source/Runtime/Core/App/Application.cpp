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
			//AHO_CORE_TRACE("{0}", m_EventManager->GetQueueSize());
			float currTime = (float)glfwGetTime();
			float deltaTime = currTime - m_LastFrameTime;
			m_LastFrameTime = currTime;
			for (auto layer : m_LayerStack) {
				layer->OnUpdate(deltaTime);
			}
			m_ImGuiLayer->Begin();
			for (auto layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			while (!m_EventManager->Empty()) {
				auto e = m_EventManager->PopFront();
				OnEvent(*e);
			}
			m_Window->OnUpdate();
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
			if (e.Handled()) {
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

