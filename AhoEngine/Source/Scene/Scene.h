#pragma once

#include "entt.hpp"

#include "Core/Camera/Camera.h"
#include "Core/Renderer/Shader.h"

namespace Aho {
	class Entity;

	class AHO_API Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		
		void OnUpdateEditor(Camera* camera, std::shared_ptr<Shader>& shader, glm::vec4& color);

		void RenderScene(Camera* camera, std::shared_ptr<Shader>& shader);

	private:
		entt::registry m_Registry;
		friend class Entity;

	};


}
