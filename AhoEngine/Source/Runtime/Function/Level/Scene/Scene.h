#pragma once

#include "entt.hpp"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include <memory>

namespace Aho {
	class Scene {
	public:
		Scene() = default;
		~Scene() = default;
		void OnUpdate(float deltaTime);
	private:
		
	};

	class EntityManager {
	public:
		EntityManager() = default;
		~EntityManager() = default;

		// TODO : override some function to support printing the name of entity/component
		template<typename T, typename... Args>
		T& AddComponent(Entity entity, Args&&... args) {
			AHO_CORE_ASSERT(!HasComponent<T>(entity), "Already has this component!");
			T& component = m_Registry.emplace<T>(entity.GetEntityHandle(), std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent(Entity entity) {
			AHO_CORE_ASSERT(HasComponent<T>(entity), "Does not have this component!");
			return m_Registry.get<T>(entity.GetEntityHandle());
		}

		template<typename T>
		bool HasComponent(Entity entity) {
			return m_Registry.all_of<T>(entity.GetEntityHandle());
		}

		Entity CreateEntity(const std::string& name = std::string()) {
			Entity entity{ m_Registry.create() };
			auto& tag = AddComponent<TagComponent>(entity, name);
			tag.Tag = name.empty() ? "IamAnEntity" : name;
			return entity;
		}
	private:
		entt::registry m_Registry;
	};
}
