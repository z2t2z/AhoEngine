#include "Ahopch.h"
#include "ResourceLayer.h"
#include "ResourceType/ResourceType.h"

namespace Aho {
	ResourceLayer::ResourceLayer(EventManager* eventManager, AssetManager* assetManager) : m_EventManager(eventManager), m_AssetManager(assetManager) {}

	void ResourceLayer::OnAttach() {
	}
	void ResourceLayer::OnDetach() {
	}
	void ResourceLayer::OnUpdate(float deltaTime) {
	}
	void ResourceLayer::OnImGuiRender() {
	}

	void ResourceLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::AssetImported) {
			e.SetHandled();
			auto ee = (AssetImportedEvent*)&(e);
			auto& path = ee->GetFilePath();
			LoadAssetFromFile(path);
		}
	}
	
	void ResourceLayer::LoadAssetFromFile(const std::string& path /*, TODO: Type assetType*/) {
		std::shared_ptr<StaticMesh> res = std::make_shared<StaticMesh>();
		m_AssetManager->LoadAssetFromFile(path, *res);
		PackRenderData(res);
	}

	template<typename T>
	void ResourceLayer::PackRenderData(const T& res) {
		std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(res);
		m_EventManager->PushBack(e);
	}
}