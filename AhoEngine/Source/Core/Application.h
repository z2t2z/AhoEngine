#pragma once

#include "Core.h"

namespace Aho {

	class AHO_API Application {
	public:
		Application();

		virtual ~Application();

		void Run();

	private:


	};

	// To be defined in the Client
	Application* CreateApplication();

} // Namespace Aho

