#pragma once

#include <memory>

#include "entt.hpp"
#include "Scene.h"

namespace Aho {
	class AHO_API Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			//AHO_CORE_ASSERT(!HasComponent<T>(), "Already has this component!");

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}
		
		template<typename T>
		T& GetComponent() {
			AHO_CORE_ASSERT(HasComponent<T>(), "Does not have this component!");

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() {
			//m_Scene->m_Registry.try_get<T>(m_EntityHandle);
			return true;
			//return m_Scene->m_Registry.has<T>(m_EntityHandle);1`
		}

		operator bool() const { return m_EntityHandle != entt::null; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
		//std::weak_ptr<Scene> m_Scene;
	};

}