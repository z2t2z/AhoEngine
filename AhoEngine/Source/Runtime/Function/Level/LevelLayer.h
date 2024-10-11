#pragma once
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Level.h"

namespace Aho {
	class LevelLayer : public Layer {
	public:
		LevelLayer(EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager);
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
	private:
		void UploadRenderDataEventTrigger(const std::vector<std::shared_ptr<RenderData>>& renderDataAll);
	private:
		bool m_SimulateMode{ false };
		bool m_PlayMode{ false };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<Level> m_CurrentLevel;
		std::shared_ptr<CameraManager> m_CameraManager;
		std::vector<std::shared_ptr<Level>> m_Levels;
	};
} // namespace Aho