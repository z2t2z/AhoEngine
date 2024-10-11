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
			auto levelLayer = new LevelLayer(eventManager, cameraManager);
			PushLayer(new RenderLayer(eventManager, renderer, cameraManager));
			PushLayer(levelLayer);
			PushLayer(new ResourceLayer(eventManager, assetManager));
			PushLayer(new AhoEditorLayer(levelLayer, eventManager, renderer, cameraManager));
		}
		~AhoEditor() {}
	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}