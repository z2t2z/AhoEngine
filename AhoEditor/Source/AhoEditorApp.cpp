#include "IamAho.h"

#include "AhoEditorLayer.h"
#include "CPURayTracing/RayTracingLayer.h"
#include "Core/EntryPoint.h"

namespace Aho {
	class AhoEditor : public Application {
	public:
		AhoEditor() {
			PushLayer(new RayTracingLayer());
		}

		~AhoEditor() {

		}

	private:

	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}