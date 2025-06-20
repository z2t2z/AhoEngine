#pragma once

#include "Entity.h"
#include "Components.h"

#include <mutex>

namespace Aho {
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

		template<typename... Components>
		std::tuple<Components*...> TryGet(Entity entity) {
			return m_Registry.try_get<Components...>(entity.GetEntityHandle());
		}

		template<typename T>
		bool HasComponent(Entity entity) {
			return m_Registry.all_of<T>(entity.GetEntityHandle());
		}

		template<typename... T>
		auto GetView() {
			return m_Registry.view<T...>();
		}

		//template<typename... Include, typename... Exclude>
		//auto GetView(Exclude ...ex) {
		//	return m_Registry.view<Include...>(ex...);
		//}

		// Need better impl
		template<typename... Include, typename Filter>
		auto GetView(Filter&& filter) {
			return m_Registry.view<Include...>(std::forward<Filter>(filter));
		}

		template <typename T>
		void RemoveComponent(Entity entity) {
			m_Registry.remove<T>(entity.GetEntityHandle());
		}

		Entity CreateEntity();

		void DestroyEntity(Entity entity);

		bool IsEntityIDValid(uint32_t id) {
			return m_Registry.valid(static_cast<entt::entity>(id));
		}
	private:
		entt::registry m_Registry;
	};
}
