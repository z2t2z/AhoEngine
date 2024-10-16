#pragma once
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Level.h"
#include "Runtime/Function/Renderer/RenderLayer.h"
#include <thread>
#include <future>

namespace Aho {
	// TODO
	struct LightData {
		glm::vec4 lightPosition[4];
		glm::vec4 lightColor[4];
		LightData() {
			for (int i = 0; i < 4; i++) {
				lightPosition[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				lightColor[i] = glm::vec4(0.1f);
			}
		}
	};

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
		LightData* GetLightData() { return &m_LightData; }
		UBO* GetUBO() { return m_RenderLayer->GetUBO(); }
	private:
		void SubmitRenderData();
		void AsyncLoadStaticMesh(const std::shared_ptr<StaticMesh> rawData) { std::thread(&LevelLayer::LoadStaticMeshAsset, this, rawData).detach(); }
		void LoadStaticMeshAsset(std::shared_ptr<StaticMesh> asset);
		void UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll);
	private:
		bool m_SimulateMode{ false };
		bool m_PlayMode{ false };
		bool m_Update{ true };		// TODO: temporary...
		LightData m_LightData;		// TODO: temporary...
	private:
		RenderLayer* m_RenderLayer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Level> m_CurrentLevel;
		std::shared_ptr<CameraManager> m_CameraManager;
		std::vector<std::shared_ptr<Level>> m_Levels;
	};
} // namespace Aho