#include "Ahopch.h"
#include "Scene.h"

#include "Core/Renderer/Renderer.h"
#include "Entity.h"
#include "Components.h"


namespace Aho {
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity { m_Registry.create(), this };
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	
	void Scene::OnUpdateRuntime(std::shared_ptr<Shader>& shader, float deltaTime) {
		// TODO
	}

	void Scene::OnUpdateEditor(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader, float deltaTime) {
		//m_CameraManager->Update(deltaTime);
		RenderScene(camera, shader);
	}

	void Scene::RenderScene(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader) {
		Renderer::BeginScene(camera);
		auto view = m_Registry.view<MeshesComponent>();
		for (const auto& e : view) {
			auto& mc = view.get<MeshesComponent>(e);
			for (const auto& e : mc.model) {
				Renderer::Submit(e);
			}
		}
		Renderer::EndScene();
	}
}