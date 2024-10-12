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
			auto renderLayer = new RenderLayer(eventManager, renderer, cameraManager);
			auto levelLayer = new LevelLayer(renderLayer, eventManager, cameraManager);
			auto resourceLayer = new ResourceLayer(eventManager, assetManager);
			auto editorLayer = new AhoEditorLayer(levelLayer, eventManager, renderer, cameraManager);
			PushLayer(renderLayer);
			PushLayer(levelLayer);
			PushLayer(resourceLayer);
			PushLayer(editorLayer);
		}
		~AhoEditor() {}
	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}