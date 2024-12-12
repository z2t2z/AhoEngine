#pragma once

#include "entt.hpp"
#include "Runtime/Core/BVH.h"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include "Ecs/EntityManager.h"
#include <memory>

namespace Aho {
	class Level {
	public:
		Level() { m_EntityManager = new EntityManager(); m_BVH = std::make_unique<BVH>(); }
		//~Level() { delete m_EntityManager; }
		EntityManager* GetEntityManager() { return m_EntityManager; }

		std::unique_ptr<BVHNode>& GetSceneBVHRoot() { return m_BVH->GetRoot(); }
		std::unique_ptr<BVH>& GetSceneBVH() { return m_BVH; }
	private:
		std::vector<Entity> m_Entities;
		std::unique_ptr<BVH> m_BVH;
		EntityManager* m_EntityManager{ nullptr };
	};
}
