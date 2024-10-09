#pragma once

#include "Runtime/Function/Level/Scene/Scene.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "entt.hpp"
#include <memory>

namespace Aho {
	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(entt::entity handle, Scene* scene, const UUID& uuid);
		Entity(const Entity& other) = default;

		// TODO : override some function to support printing the name of entity/component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			AHO_CORE_ASSERT(!HasComponent<T>(), "Already has this component!");
			T& component = m_Scene->m_EntityManager.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent() {
			AHO_CORE_ASSERT(HasComponent<T>(), "Does not have this component!");
			return m_Scene->m_EntityManager.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_EntityManager.all_of<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }

		UUID GetAssetUUID() { return m_AssetUUID; }
		entt::entity GetEntityHandle() { return m_EntityHandle; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene{ nullptr };
		UUID m_AssetUUID;
	};
}
