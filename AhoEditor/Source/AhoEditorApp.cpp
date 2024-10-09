#include "IamAho.h"
#include "AhoEditorLayer.h"
#include "CPURayTracing/RayTracingLayer.h"
#include "Runtime/Core/App/EntryPoint.h"

namespace Aho {
	class AhoEditor : public Application {
	public:
		AhoEditor() {
			//PushLayer(new RayTracingLayer());

			Renderer* renderer = new Renderer();
			auto cameraManager = std::make_shared<CameraManager>();
			cameraManager->GetMainEditorCamera()->MoveBackward(1.0f);
			PushLayer(new LevelLayer(cameraManager));
			PushLayer(new RenderLayer(renderer, cameraManager));
			PushLayer(new AhoEditorLayer(renderer, cameraManager));
		}
		~AhoEditor() {}
	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}