#include "Ahopch.h"
#include "Scene.h"

#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Resource/EcS/AObject.h"
#include "Runtime/Resource/Ecs/Components.h"


namespace Aho {
	AObject Scene::CreateAObject(const std::string& name) {
		AObject entity { m_Registry.create(), this };
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
		auto view = m_Registry.view<MeshesComponent>();
		for (const auto& e : view) {
			auto& mc = view.get<MeshesComponent>(e);
			for (size_t i = 0; i < mc.meshAsset->m_VAOs.size(); i++) {
				if (i < mc.meshAsset->m_Materials.size()) {
					mc.meshAsset->m_Materials[i]->GetMaterial()->Apply(shader);
				}
				Renderer::Submit(mc.meshAsset->m_VAOs[i]);
			}
			//for (const auto& e : *mc.meshAsset) {
			//	Renderer::Submit(e);
			//}
			//Renderer::Submit(mc.vertexArray);
			//for (const auto& e : mc.model) {
			//	Renderer::Submit(e);
			//}
		}
		Renderer::EndScene();
	}
}