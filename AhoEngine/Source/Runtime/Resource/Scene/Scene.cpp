#include "Ahopch.h"
#include "Scene.h"

#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Resource/EcS/AObject.h"
#include "Runtime/Resource/Ecs/Components.h"


namespace Aho {
	AObject Scene::CreateAObject(const std::string& name) {
		AObject entity { m_Registry.create(), this, };
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "IamAnAObject" : name;
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
		//shader->Bind();
		Renderer::BeginScene(camera);
		auto view = m_Registry.view<EntityComponent>();
		for (const auto& entity : view) {
			auto& meshentities = m_Registry.get<EntityComponent>(entity);
			for (const auto& meshEntity : meshentities.meshEntities) {
				auto& mesh = m_Registry.get<MeshComponent>(meshEntity);
				auto materialComponent = m_Registry.try_get<MaterialComponent>(meshEntity);
				if (materialComponent && materialComponent->material) {
					materialComponent->material->Apply(shader);
				}

				mesh.vertexArray->Bind();
				Renderer::Submit(mesh.vertexArray);
				mesh.vertexArray->Unbind();
				if (materialComponent && materialComponent->material) {
					materialComponent->material->Unbind(shader);
				}
			}
		}
		Renderer::EndScene();
	}
}