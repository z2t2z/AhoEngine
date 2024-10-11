#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "entt.hpp"
#include <memory>

namespace Aho {
	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle);
		Entity(entt::entity handle, const UUID& uuid);
		Entity(const Entity& other) = default;
		operator bool() const { return m_EntityHandle != entt::null; }
		UUID GetAssetUUID() { return m_AssetUUID; }
		entt::entity GetEntityHandle() { return m_EntityHandle; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		UUID m_AssetUUID;
	};
}
