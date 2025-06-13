#pragma once

#include "Runtime/Function/Level/EcS/Entity.h"

namespace Aho {
	class EditorGlobalContext {
	public:
		bool HasActiveSelected() const { return m_Selected.Valid(); }
		const Entity& GetSelectedEntity() const;
		void RequestPicking(int x, int y);
		void SetSelected(Entity selected);
		void SetSelected(uint32_t id);
		std::tuple<bool, int, int> PickInfoAtThisFrame() const { return m_PickInfo; }
	private:
		std::tuple<bool, int, int> m_PickInfo = { false, -1, -1 };
		Entity m_Selected;
	};

	inline EditorGlobalContext g_EditorGlobalCtx;
}
