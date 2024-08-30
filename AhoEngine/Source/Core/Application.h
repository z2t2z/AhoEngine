#pragma once

#include "Core/Core.h"
#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Layer/LayerStack.h"

namespace Aho {

	class AHO_API Application {
	public:
		Application();

		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;

		bool m_Running = true;

		LayerStack m_LayerStack;

	};

	// To be defined in the Client
	Application* CreateApplication();

} // Namespace Aho

