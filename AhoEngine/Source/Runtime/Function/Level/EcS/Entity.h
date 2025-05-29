#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "entt.hpp"
#include <memory>

namespace Aho {
	template<typename... E>
	inline constexpr entt::exclude_t<E...> Exclude{};

	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle);
		Entity(entt::entity handle, const UUID& uuid);
		Entity(const Entity& other) = default;
		UUID GetAssetUUID() { return m_AssetUUID; }
		entt::entity GetEntityHandle() const { return m_EntityHandle; }
		bool Valid() const { return m_EntityHandle != entt::null; }
		void SetInvalid() { m_EntityHandle = entt::null; }
	public:
		operator bool() const { return m_EntityHandle != entt::null; }
		bool operator==(const Entity& other) const { return other.m_EntityHandle == m_EntityHandle; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		UUID m_AssetUUID;
	};
}
