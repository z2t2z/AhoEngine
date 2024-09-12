#pragma once

#include "entt.hpp"

namespace Aho {
	class Entity;

	class AHO_API Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		
		void OnUpdate();

	private:
		entt::registry m_Registry;
		friend class Entity;

	};


}
