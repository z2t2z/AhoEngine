#include "Ahopch.h"
#include "Application.h"


namespace Aho {

// std::bind(): Extract a member function from a class as a new function that can be directly called, while binding some parameters in advance
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application() {

	}
	
	void Application::Run() {

		while (m_Running) {
			for (auto layer : m_LayerStack) {
				layer->OnUpdate();
			}

			m_Window->OnUpdate();
		}

	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		AHO_CORE_TRACE("{0}", e.ToString());			 			// ?

		for (auto it = std::prev(m_LayerStack.end()); ; it--) {
			(*it)->OnEvent(e);
			if (e.Handled()) {
				break;
			}

			if (it == m_LayerStack.begin()) {
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

} // Namespace IamAho

