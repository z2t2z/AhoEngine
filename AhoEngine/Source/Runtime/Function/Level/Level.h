#pragma once

#include "entt.hpp"
#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include "Ecs/EntityManager.h"
#include <memory>

namespace Aho {
	class Level {
	public:
		Level() { 
			m_EntityManager = new EntityManager(); 
			m_EnvEntity = Entity();
			m_EntityManager->AddComponent<GameObjectComponent>(m_EnvEntity, "Environment Map");
		}
		//~Level() { delete m_EntityManager; }
		EntityManager* GetEntityManager() { return m_EntityManager; }
		BVHi& GetTLAS() { return m_TlasBvh; }
		Entity GetEnvEntity() const { return m_EnvEntity; }
	private:
		Entity m_EnvEntity;
		BVHi m_TlasBvh;
		std::vector<Entity> m_Entities;
		EntityManager* m_EntityManager{ nullptr };
	};
}
