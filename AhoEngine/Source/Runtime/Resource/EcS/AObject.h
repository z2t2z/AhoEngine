#pragma once

#include "Runtime/Resource/Scene/Scene.h"
#include "Runtime/Resource/UUID/UUID.h"
#include "entt.hpp"
#include <memory>

namespace Aho {
	class AHO_API AObject {
	public:
		AObject() = default;
		AObject(entt::entity handle, Scene* scene);
		AObject(entt::entity handle, Scene* scene, const UUID& uuid);
		AObject(const AObject& other) = default;
		
		// TODO : override some function to support printing the name of entity/component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			AHO_CORE_ASSERT(!HasComponent<T>(), "Already has this component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_AObjectHandle, std::forward<Args>(args)...);
			return component;
		}
		
		template<typename T>
		T& GetComponent() {
			AHO_CORE_ASSERT(HasComponent<T>(), "Does not have this component!");
			return m_Scene->m_Registry.get<T>(m_AObjectHandle);
		}

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_AObjectHandle);
		}

		operator bool() const { return m_AObjectHandle != entt::null; }

		UUID GetAssetUUID() { return m_AssetUUID; }
		entt::entity GetEntityHandle() { return m_AObjectHandle; }
	private:
		entt::entity m_AObjectHandle{ entt::null };
		Scene* m_Scene{ nullptr };
		UUID m_AssetUUID;
	};
}