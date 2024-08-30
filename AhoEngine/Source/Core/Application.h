#pragma once

#include "Core/Core.h"
#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Layer/LayerStack.h"

namespace Aho {

	class AHO_API Application {
	public:
		//static Application* s_Instance;

		Application();

		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() {
			return *s_Instance;
		}
		
		inline Window& GetWindow() {
			return *m_Window;
		}

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;

		bool m_Running = true;

		LayerStack m_LayerStack;

		static Application* s_Instance;

	};

	// To be defined in the Client
	Application* CreateApplication();

} // Namespace Aho

