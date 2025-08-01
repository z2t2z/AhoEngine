#pragma once

#include "Runtime/Core/Layer/Layer.h"
#include <memory>
#include <thread>
#include <future>

namespace Aho {
	struct alignas(16) MaterialDescriptor;
	class Texture;
	class RenderLayer;
	class CameraManager;
	class Level;
	class StaticMesh;
	class SkeletalMesh;
	class Light;

	class LevelLayer : public Layer {
	public:
		LevelLayer(RenderLayer* renderLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~LevelLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
		std::shared_ptr<Level> GetCurrentLevel() { return m_CurrentLevel; }
		void AddLevel(const std::shared_ptr<Level>& scene) { m_Levels.push_back(scene); }
		void SetPlayMode(bool state) { m_PlayMode = state; }
		void SetSimulateMode(bool state) { m_SimulateMode = state; }
		MaterialDescriptor& GetMaterialDescriptor(int meshId) { return m_TextureHandles.at(meshId); }
		void AddStaticMeshToScene(const std::shared_ptr<StaticMesh>& asset, const std::string& name, const std::shared_ptr<Light>& light = nullptr);
	private:
		void UpdateAnimation(float deltaTime);
		void AddAnimation(const std::shared_ptr<AnimationAsset>& anim);
		void UpdataUBOData();
		void AsyncLoadStaticMesh(const std::shared_ptr<StaticMesh> rawData) { std::thread(&LevelLayer::LoadStaticMeshAsset, this, rawData).detach(); }
		void LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset);
		void LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset);
		void UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll);
	private:
		bool m_SimulateMode{ false };
		bool m_PlayMode{ false };
		bool m_Update{ true };		// TODO: temporary...
	private:
		inline static int m_SkeletalMeshBoneOffset{ 0 };
	private:
		bool m_BuildBVH{ true };
		RenderLayer* m_RenderLayer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Level> m_CurrentLevel{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
		std::vector<std::shared_ptr<Level>> m_Levels;
	// temporary
	private:
		inline static int s_globalSubMeshId{ 0 };
		std::vector<MaterialDescriptor> m_TextureHandles;
	};
} // namespace Aho