#pragma once

#include "Core/Core.h"
#include "Window.h"

namespace Aho {

	class AHO_API Application {
	public:
		Application();

		virtual ~Application();

		void Run();

	private:
		std::unique_ptr<Window> m_Window;

		bool m_Running = true;

	};

	// To be defined in the Client
	Application* CreateApplication();

} // Namespace Aho

