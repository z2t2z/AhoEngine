#include "Ahopch.h"
#include "Application.h"

#include "Core/Input/Input.h"

#include "Core/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"


namespace Aho {

// std::bind(): Extract a member function from a class as a new function that can be directly called, while binding some parameters in advance
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		AHO_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::Run() {

		while (m_Running) {

			for (auto layer : m_LayerStack) {
				layer->OnUpdate();
			}
			
			// Will be done on the render thread in the future
			m_ImGuiLayer->Begin();
			for (auto layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}

	}

	void Application::ShutDown() {
		m_Running = false;
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		/*   Log every event here in the console */
		//AHO_CORE_TRACE("{0}", e.ToString());			 			// ?

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

