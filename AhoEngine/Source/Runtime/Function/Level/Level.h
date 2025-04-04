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
		}
		//~Level() { delete m_EntityManager; }
		EntityManager* GetEntityManager() { return m_EntityManager; }

		BVHi& GetTLAS() { return m_TlasBvh; }

	private:
		BVHi m_TlasBvh;
		std::vector<Entity> m_Entities;
		EntityManager* m_EntityManager{ nullptr };
	};
}
