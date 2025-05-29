#include "IamAho.h"
#include "AhoEditorLayer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "CPURayTracing/RayTracingLayer.h"
#include "Runtime/Core/App/EntryPoint.h"

namespace Aho {
	class AhoEditor : public Application {
	public:
		AhoEditor() {
			g_RuntimeGlobalCtx.InitializeSystems("");
			Renderer* renderer = g_RuntimeGlobalCtx.m_Renderer.get();

			auto cameraManager = std::make_shared<CameraManager>();
			cameraManager->GetMainEditorCamera()->MoveBackward(10.0f);

			AssetManager* assetManager = g_RuntimeGlobalCtx.m_AssetManager.get();

			auto eventManager = GetEventManager();
			auto renderLayer = new RenderLayer(eventManager, renderer, cameraManager);
			auto levelLayer = new LevelLayer(renderLayer, eventManager, cameraManager);
			auto editorLayer = new AhoEditorLayer(levelLayer, eventManager, renderer, cameraManager);
			PushLayer(renderLayer);
			PushLayer(levelLayer);
			PushLayer(editorLayer);
		}
		~AhoEditor() {}
	};

	Application* CreateApplication() {
		return new AhoEditor();
	}
}