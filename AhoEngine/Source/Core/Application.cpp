#include "Ahopch.h"
#include "Application.h"




namespace Aho {

// std::bind(): Extract a member function from a class as a new function that can be directly called, while binding some parameters in advance
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		AHO_CORE_TRACE("{0}", e.ToString());			 			// ?
	}

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application() {

	}
	
	void Application::Run() {

		while (m_Running) {
			m_Window->OnUpdate();
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

} // Namespace IamAho

