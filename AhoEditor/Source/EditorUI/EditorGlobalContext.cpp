#include "EditorGlobalContext.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

namespace Aho {
	EditorGlobalContext g_EditorGlobalCtx;

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

	const Entity& EditorGlobalContext::GetSelectedEntity() const {
		return m_Selected;
	}
}
