#pragma once

#include "entt.hpp"

#include "Core/Renderer/Shader.h"
#include "Core/Camera/CameraManager.h"
#include <memory>

namespace Aho {
	class Entity;

	class Scene {
	public:
		Scene() = default;
		Scene(std::shared_ptr<CameraManager> cameraManager) 
			: m_CameraManager(std::move(cameraManager)) {}
		~Scene() = default;

		Entity CreateEntity(const std::string& name = std::string());
		
		void OnUpdateRuntime(std::shared_ptr<Shader>& shader, float deltaTime);
		void OnUpdateEditor(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader, float deltaTime);

		void RenderScene(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader);

	private:
		friend class Entity;
		std::shared_ptr<CameraManager> m_CameraManager;
		entt::registry m_Registry;
	};
}
