#include "Ahopch.h"
#include "ResourceLayer.h"
#include "ResourceType/ResourceType.h"

namespace Aho {
	namespace fs = std::filesystem;
	ResourceLayer::ResourceLayer(EventManager* eventManager, AssetManager* assetManager) 
		: Layer("ResourceLayer"), m_EventManager(eventManager), m_AssetManager(assetManager) {
		auto path = fs::current_path() / "Asset" / "Basic";
		m_Cube = std::make_shared<StaticMesh>();
		m_Sphere = std::make_shared<StaticMesh>();
		m_Cylinder = std::make_shared<StaticMesh>();
		m_AssetManager->LoadAssetFromFile(path / "Cube.fbx", *m_Cube);
		m_AssetManager->LoadAssetFromFile(path / "Sphere.fbx", *m_Sphere);
		m_AssetManager->LoadAssetFromFile(path / "Cylinder.fbx", *m_Cylinder);
	}

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
			AHO_CORE_WARN("Recieving a AssetImportedEvent!");
			auto& path = ee->GetFilePath();
			LoadAssetFromFile(path);
		}
	}
	
	void ResourceLayer::LoadAssetFromFile(const std::string& path /*, TODO: Type assetType*/) {
		std::shared_ptr<StaticMesh> res = std::make_shared<StaticMesh>();
		m_AssetManager->LoadAssetFromFile(path, *res);
		m_AssetManager->AddAsset(path, res->GetUUID(), res);
		PackRenderData(res);
		//PackEcSData(res);
	}

	template<typename T>
	void ResourceLayer::PackEcSData(const T& res) {
		//std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(res);
		//m_EventManager->PushBack(e);
	}

	template<typename T>
	void ResourceLayer::PackRenderData(const T& res) {
		std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(res);
		AHO_CORE_WARN("Pushing a PackRenderDataEvent!");  // pass to levelLayer
		m_EventManager->PushBack(e);
	}
}