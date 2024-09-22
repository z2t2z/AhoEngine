#pragma once

#include "Core/Core.h"
#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"

#include "Layer/LayerStack.h"
#include "Core/Gui/ImGuiLayer.h"


namespace Aho {
	class AHO_API Application {
	public:
		Application();
		virtual ~Application() = default;

		void Run();

		void ShutDown();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::shared_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be defined in the Client
	Application* CreateApplication();

} // namespace Aho

