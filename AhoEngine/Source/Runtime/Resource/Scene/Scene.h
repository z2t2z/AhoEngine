#pragma once

#include "entt.hpp"

#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Function/Camera/CameraManager.h"
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

		void SecondPass(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader);
		entt::registry m_Registry;
	private:
		friend class AObject;
	};
}
