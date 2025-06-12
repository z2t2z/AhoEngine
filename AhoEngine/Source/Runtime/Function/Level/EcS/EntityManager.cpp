#include "Ahopch.h"
#include "EntityManager.h"
#include "Components.h"

namespace Aho {
	Entity EntityManager::CreateEntity() {
		Entity entity{ m_Registry.create() };
		if (static_cast<uint32_t>(entity.GetEntityHandle()) == 0u) {
			entity = m_Registry.create();
		}
		return entity;
	}

	void EntityManager::DestroyEntity(Entity entity) {
		AHO_CORE_ASSERT(entity.Valid(), "Trying to destroy an invalid entity!");
		if (HasComponent<GameObjectComponent>(entity)) {
			auto goComp = GetComponent<GameObjectComponent>(entity);
			for (auto child : goComp.children) {
				DestroyEntity(child);
			}
		}
		m_Registry.destroy(entity.GetEntityHandle());
	}
}