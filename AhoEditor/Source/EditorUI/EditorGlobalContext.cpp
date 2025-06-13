#include "EditorGlobalContext.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

namespace Aho {
	void EditorGlobalContext::SetSelected(Entity selected) {
		if (m_Selected == selected)
			return;
		
		m_Selected = selected;
		return;

		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto view = ecs->GetView<EditorSelectedTag>();
		view.each(
			[&](Entity entity, const EditorSelectedTag& tag) {
				if (entity != m_Selected)
					ecs->RemoveComponent<EditorSelectedTag>(entity);
			}
		);
		if (!ecs->HasComponent<EditorSelectedTag>(m_Selected)) {
			ecs->AddComponent<EditorSelectedTag>(m_Selected);
		}
	}

	void EditorGlobalContext::RequestPicking(int x, int y) {
		m_PickInfo = { true, x, y };
	}

	void EditorGlobalContext::SetSelected(uint32_t id) {
		Entity e = Entity(entt::entity(id));
		if (e.Valid()) {
			m_Selected = e;
		}
		m_PickInfo = { false, 0, 0 };
	}


	const Entity& EditorGlobalContext::GetSelectedEntity() const {
		return m_Selected;
	}
}
