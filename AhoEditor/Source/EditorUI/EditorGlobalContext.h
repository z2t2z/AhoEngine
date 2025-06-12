#pragma once

#include "Runtime/Function/Level/EcS/Entity.h"

namespace Aho {
	class EditorGlobalContext {
	public:
		bool HasActiveSelected() const { return m_Selected.Valid(); }
		const Entity& GetSelectedEntity() const;
		void SetSelected(Entity selected);
	private:
		uint64_t m_Frame{ 0 };
		Entity m_Selected;
	};

	extern EditorGlobalContext g_EditorGlobalCtx;
}
