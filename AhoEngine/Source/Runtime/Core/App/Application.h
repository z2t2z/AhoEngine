#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Window/Window.h"
#include "Runtime/Core/Events/ApplicationEvent.h"
#include "Runtime/Core/Events/Event.h"
#include "Runtime/Core/Layer/LayerStack.h"
#include "Runtime/Core/Gui/ImGuiLayer.h"

namespace Aho {
	class Application {
	public:
		Application();
		virtual ~Application() { delete m_EventManager; }
		void Run();
		void ShutDown();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	protected:
		EventManager* GetEventManager() { return m_EventManager; }
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		uint32_t m_FPS{ 0u };
		float m_AccumulatedTime{ 0.0f };
		bool m_Running = true;
		float m_LastFrameTime = 0.0f;
	private:
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer{ nullptr };
		LayerStack m_LayerStack;
	private:
		static Application* s_Instance;
	};

	// To be defined in the Client
	Application* CreateApplication();

} // namespace Aho

