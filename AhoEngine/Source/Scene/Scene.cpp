#include "Ahopch.h"
#include "Scene.h"

#include "Core/Renderer/Renderer.h"
#include "Entity.h"
#include "Components.h"


namespace Aho {
	Scene::Scene() {

	}

	Scene::~Scene() {

	}

	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity { m_Registry.create(), this };
		//entity.AddComponent<TransformComponent>();
		//auto& tag = entity.AddComponent<TagComponent>();
		//tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	
	void Scene::OnUpdateEditor(Camera* camera, std::shared_ptr<Shader>& shader, glm::vec4& color) {
		const auto& view = m_Registry.view<CameraComponent, TransformComponent>();
		glm::mat4* transform = nullptr;
		Camera* mainCamera = nullptr;
		for (const auto& e : view) {
			auto cc = view.get<CameraComponent>(e);
			if (cc.Primary) {
				mainCamera = cc.camera;
				auto tct = view.get<TransformComponent>(e).GetTransform();
				transform = &tct;
				break;
			}
		}
		AHO_ASSERT(transform != nullptr, "Doesn't have a main camera!");
		auto mat = *transform;

		Renderer::BeginScene(camera, *transform, color);
		auto v = glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
		AHO_TRACE("({0}, {1}, {2})", v.x, v.y, v.z);
		{
			auto view = m_Registry.group<MeshComponent, TransformComponent>();
			for (const auto& e : view) {
				auto& mc = view.get<MeshComponent>(e);
				Renderer::Submit(mc.vertexArray);
			}
		}
		Renderer::EndScene();
	}

	void Scene::RenderScene(Camera* camera, std::shared_ptr<Shader>& shader) {


	}
}