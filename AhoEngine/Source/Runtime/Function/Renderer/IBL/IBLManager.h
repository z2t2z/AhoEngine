#pragma once

#include "Runtime/Function/Level/EcS/Entity.h"

namespace Aho {
	// Keep it simple for now
	class Shader;
	class IBLManager {
	public:
		void SetActiveIBL(Entity entity) {
			m_ActiveIBLEntity = entity;
		}
		Entity GetActiveIBL() const {
			return m_ActiveIBLEntity;
		}
		void BindActiveIBL(const Shader* shader) const; // For PT
		void BindActiveIBLDeferred(const Shader* shader, uint32_t& slot) const; // For deferred

	private:
		Entity m_ActiveIBLEntity;
	};
}
