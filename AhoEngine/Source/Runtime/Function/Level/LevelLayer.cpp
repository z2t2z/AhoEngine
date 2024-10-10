#include "Ahopch.h"
#include "LevelLayer.h"

namespace Aho {
	LevelLayer::LevelLayer(EventManager* eventManager, const std::shared_ptr<CameraManager>& cameraManager)
		: m_EventManager(eventManager), m_CameraManager(cameraManager) {
	}

	void LevelLayer::OnAttach() {
	
	}
	
	void LevelLayer::OnDetach() {
	}
	
	void LevelLayer::OnUpdate(float deltaTime) {
		if (!m_PlayMode) {
			return;
		}
		m_CurrentScene->OnUpdate(deltaTime);
		/* Actual in-game logic here */
	}
	
	void LevelLayer::OnImGuiRender() {
	}
	
	void LevelLayer::OnEvent(Event& e) {
	}
} // namespace Aho