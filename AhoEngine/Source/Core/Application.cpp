#include "Ahopch.h"
#include "Application.h"


namespace Aho {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {

	}
	
	void Application::Run() {

		while (m_Running) {
			m_Window->OnUpdate();
		}

	}


} // Namespace IamAho

