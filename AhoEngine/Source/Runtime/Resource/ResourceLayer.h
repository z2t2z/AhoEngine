#pragma once

#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "FileWatcher/FileWatcher.h"

namespace Aho {
	class ResourceLayer : public Layer {
	public:
		ResourceLayer(EventManager* eventManager, AssetManager* m_AssetManager);
		virtual ~ResourceLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
		std::shared_ptr<StaticMesh> GetCube() { return m_Cube; }
		std::shared_ptr<StaticMesh> GetSphere() { return m_Sphere; }
		std::shared_ptr<StaticMesh> GetCylinder() { return m_Cylinder; }
		std::shared_ptr<StaticMesh> GetBone() { return m_Bone; }
		std::shared_ptr<StaticMesh> GetPlane() { return m_Plane; }
	private:
		void LoadAssetFromFile(const std::string& path, bool isSkeletal, const glm::mat4& preTransform = glm::mat4(1.0f));
		template<typename T>
		void PackRenderData(const T& res, bool isSkeletal);
		void PackAnimation(const std::shared_ptr<AnimationAsset>& animation);
	private:
		AssetManager* m_AssetManager;
		EventManager* m_EventManager;
	private:
		std::shared_ptr<StaticMesh> m_Bone{ nullptr };
		std::shared_ptr<StaticMesh> m_Cube{ nullptr };
		std::shared_ptr<StaticMesh> m_Plane{ nullptr };
		std::shared_ptr<StaticMesh> m_Sphere{nullptr};
		std::shared_ptr<StaticMesh> m_Cylinder{nullptr};
	};
}