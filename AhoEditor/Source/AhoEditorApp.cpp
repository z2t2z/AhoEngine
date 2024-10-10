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
			AssetManager* assetManager = new AssetManager();
			auto eventManager = GetEventManager();
			AHO_ASSERT(eventManager);
			PushLayer(new RenderLayer(eventManager, renderer, cameraManager));
			PushLayer(new ResourceLayer(eventManager, assetManager));
			PushLayer(new LevelLayer(eventManager, cameraManager));
			PushLayer(new AhoEditorLayer(eventManager, renderer, cameraManager));
		}
		~AhoEditor() {}
	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}