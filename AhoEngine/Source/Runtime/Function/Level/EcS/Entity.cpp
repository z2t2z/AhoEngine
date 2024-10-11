#include "Ahopch.h"
#include "Entity.h"

namespace Aho {
	Entity::Entity(entt::entity handle) : m_EntityHandle(handle) {

	}
	Entity::Entity(entt::entity handle, const UUID& uuid) : m_EntityHandle(handle), m_AssetUUID(uuid) {

	}
}