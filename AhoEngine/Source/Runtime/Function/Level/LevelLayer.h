#pragma once
#include "Runtime/Core/Layer/Layer.h"
#include "Scene/Scene.h"

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

		std::shared_ptr<Scene> GetCurrentScene() { return m_CurrentScene; }
		void AddScene(const std::shared_ptr<Scene>& scene) { m_Scenes.push_back(scene); }
	private:
		bool m_SimulateMode{ false };
		bool m_PlayMode{ false };
		EventManager* m_EventManager;
		std::shared_ptr<Scene> m_CurrentScene;
		std::shared_ptr<CameraManager> m_CameraManager;
		std::vector<std::shared_ptr<Scene>> m_Scenes;
	};
} // namespace Aho