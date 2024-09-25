#include "IamAho.h"

#include "AhoEditorLayer.h"
#include "CPURayTracing/RayTracingLayer.h"
#include "Core/EntryPoint.h"

namespace Aho {
	class AhoEditor : public Application {
	public:
		AhoEditor() {
			PushLayer(new RayTracingLayer());
			//PushLayer(new AhoEditorLayer());
		}

		~AhoEditor() {

		}

	private:

	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}