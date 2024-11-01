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
		m_Plane = AssetCreator::MeshAssetCreater((path / "Floor.obj").string());
		m_AssetManager->LoadAssetFromFile(path / "Cube.obj", *m_Cube);
		m_AssetManager->LoadAssetFromFile(path / "Sphere.obj", *m_Sphere);
		m_AssetManager->LoadAssetFromFile(path / "Cylinder.obj", *m_Cylinder);
		m_Bone = AssetCreator::MeshAssetCreater((path / "DownBone.obj").string());
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
			LoadAssetFromFile(path, ee->IsStaticMesh());
		}
	}
	
	/* BIG TODO */
	void ResourceLayer::LoadAssetFromFile(const std::string& path, bool isSkeletal) {
		if (isSkeletal) {
			auto res = AssetCreator::SkeletalMeshAssetCreator(path);
			auto anim = AssetCreator::AnimationAssetCreator(path, res);
			if (!anim) {
				AHO_CORE_WARN("Skeletal mesh does not have animation data at path {}", path);
			}
			PackRenderData(res, true);
			PackAnimation(anim);
		}
		else {
			std::shared_ptr<StaticMesh> res = std::make_shared<StaticMesh>();
			m_AssetManager->LoadAssetFromFile(path, *res);
			m_AssetManager->AddAsset(path, res->GetUUID(), res);
			PackRenderData(res, false);
		}
		//PackEcSData(res);
	}

	void ResourceLayer::PackAnimation(const std::shared_ptr<AnimationAsset>& animation) {
		std::shared_ptr<UploadAnimationDataEvent> e = std::make_shared<UploadAnimationDataEvent>(animation);
		AHO_CORE_WARN("Pushing a UploadAnimationDataEvent!");  // pass to levelLayer
		m_EventManager->PushBack(e);
	}

	template<typename T>
	void ResourceLayer::PackRenderData(const T& res, bool isSkeletal) {
		std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(res, isSkeletal);
		AHO_CORE_WARN("Pushing a PackRenderDataEvent!");  // pass to levelLayer
		m_EventManager->PushBack(e);
	}
}