#pragma once

#include "entt.hpp"

#include "Core/Renderer/Shader.h"
#include "Core/Camera/CameraManager.h"
#include <memory>

namespace Aho {
	class AObject;

	class Scene {
	public:
		Scene() = default;
		~Scene() = default;

		AObject CreateAObject(const std::string& name = std::string());
		
		void OnUpdateRuntime(std::shared_ptr<Shader>& shader, float deltaTime);
		void OnUpdateEditor(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader, float deltaTime);

		void RenderScene(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader);

	private:
		friend class AObject;
		entt::registry m_Registry;
	};
}
