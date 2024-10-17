#pragma once
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/RenderLayer.h"
#include "Runtime/Resource/ResourceLayer.h"
#include "Level.h"
#include <thread>
#include <future>

namespace Aho {
	struct LightData {
		int lightCnt{ 0 };
		glm::vec4 lightPosition[MAX_LIGHT_CNT];
		glm::vec4 lightColor[MAX_LIGHT_CNT];
		LightData() {
			for (int i = 0; i < MAX_LIGHT_CNT; i++) {
				lightPosition[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				lightColor[i] = glm::vec4(0.1f);
			}
		}
	};

	class LevelLayer : public Layer {
	public:
		LevelLayer(RenderLayer* renderLayer, ResourceLayer* resourceLayer, EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager);
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
		LightData* GetLightData() { return &m_LightData; }
	private:
		void AddLightSource(LightType lt);
		void SubmitUBOData();
		void AsyncLoadStaticMesh(const std::shared_ptr<StaticMesh> rawData) { std::thread(&LevelLayer::LoadStaticMeshAsset, this, rawData).detach(); }
		void LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset);
		void LoadSkeletalMeshAsset(std::shared_ptr<SkeletalMesh> asset);
		void UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll);
	private:
		bool m_SimulateMode{ false };
		bool m_PlayMode{ false };
		bool m_Update{ true };		// TODO: temporary...
		LightData m_LightData;		// TODO: temporary...
	private:
		RenderLayer* m_RenderLayer{ nullptr };
		ResourceLayer* m_ResourceLayer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Level> m_CurrentLevel{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
		std::vector<std::shared_ptr<Level>> m_Levels;
	};
} // namespace Aho