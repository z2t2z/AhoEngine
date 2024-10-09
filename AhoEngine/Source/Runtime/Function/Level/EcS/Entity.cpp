#include "Ahopch.h"
#include "Entity.h"

namespace Aho {
	Entity::Entity(entt::entity handle, Scene* scene, const UUID& uuid) : m_EntityHandle(handle), m_Scene(scene), m_AssetUUID(uuid) {

	}
	Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {

	}
}