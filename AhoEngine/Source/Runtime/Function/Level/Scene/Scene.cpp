#include "Ahopch.h"
#include "Scene.h"

#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/Ecs/Components.h"

namespace Aho {
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity { m_EntityManager.create(), this, };
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "IamAnEntity" : name;
		return entity;
	}
	void Scene::OnUpdate(float deltaTime) {

	}
}