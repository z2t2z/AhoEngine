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

	void Scene::SecondPass(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader) {
		//Renderer::Init(shader);
		//Renderer::BeginScene(camera);
		//auto view = m_Registry.view<EntityComponent>();
		//for (const auto& entity : view) {
		//	auto& meshentities = Scene::m_Registry.get<EntityComponent>(entity);
		//	for (const auto& meshEntity : meshentities.meshEntities) {
		//		auto& mesh = m_Registry.get<MeshComponent>(meshEntity);
		//		shader->SetInt("u_ID", mesh.meshID);
		//		mesh.vertexArray->Bind();
		//		Renderer::Submit(mesh.vertexArray);
		//		mesh.vertexArray->Unbind();
		//	}
		//}
		//Renderer::EndScene();
	}

	void Scene::OnUpdateEditor(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader, float deltaTime) {
		if (deltaTime == -1.0f) {
			SecondPass(camera, shader);
		}
		else {
			RenderScene(camera, shader);
		}
	}

	void Scene::RenderScene(std::shared_ptr<Camera> camera, std::shared_ptr<Shader>& shader) {
		//Renderer::Init(shader);
		//Renderer::BeginScene(camera);
		//auto view = m_Registry.view<EntityComponent>();
		//for (const auto& entity : view) {
		//	auto& meshentities = m_Registry.get<EntityComponent>(entity);
		//	for (const auto& meshEntity : meshentities.meshEntities) {
		//		auto& mesh = m_Registry.get<MeshComponent>(meshEntity);
		//		auto materialComponent = m_Registry.try_get<MaterialComponent>(meshEntity);
		//		if (materialComponent && materialComponent->material) {
		//			materialComponent->material->Apply(shader);
		//		}
		//		mesh.vertexArray->Bind();
		//		Renderer::Submit(mesh.vertexArray);
		//		mesh.vertexArray->Unbind();
		//		if (materialComponent && materialComponent->material) {
		//			materialComponent->material->Unbind(shader);
		//		}
		//	}
		//}
		//Renderer::EndScene();
	}
}