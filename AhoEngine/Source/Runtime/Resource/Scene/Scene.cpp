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
		shader->Bind();
		Renderer::BeginScene(camera);
		auto view = m_Registry.view<MultiMeshComponent>();
		for (const auto& e : view) {
			auto& meshComponent = m_Registry.get<MultiMeshComponent>(e);
			for (const auto& mc : meshComponent.meshes) {
				Renderer::Submit(mc.vertexArray);
			}
		}
		Renderer::EndScene();
	}
}