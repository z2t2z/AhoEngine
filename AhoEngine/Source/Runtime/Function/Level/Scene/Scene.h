#pragma once

#include "entt.hpp"

#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include <memory>

namespace Aho {
	class Entity;

	class Scene {
	public:
		Scene() = default;
		~Scene() = default;
		Entity CreateEntity(const std::string& name = std::string());
		void OnUpdate(float deltaTime);
	private:
		entt::registry m_EntityManager;
		friend class Entity;
	};
}
