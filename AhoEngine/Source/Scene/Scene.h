#pragma once

#include "entt.hpp"

#include "Core/Camera/CameraManager.h"
#include "Core/Renderer/Shader.h"

namespace Aho {
	class Entity;

	class AHO_API Scene {
	public:
		Scene(CameraManager& cameraManager);
		~Scene() = default;

		Entity CreateEntity(const std::string& name = std::string());
		
		void OnUpdateRuntime(std::shared_ptr<Shader>& shader);
		void OnUpdateEditor(std::shared_ptr<Shader>& shader);

		void RenderScene(std::shared_ptr<Shader>& shader);

	private:
		CameraManager& m_CameraManager;
		entt::registry m_Registry;
		friend class Entity;

	};


}
