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
		Level() { 
			m_EntityManager = new EntityManager(); 
			m_EnvEntity = Entity(m_EntityManager->CreateEntity("Environment"));
			m_EntityManager->AddComponent<RootComponent>(m_EnvEntity);
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
