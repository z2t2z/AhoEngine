#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArrayr.h"
#include "RenderCommand.h"


namespace Aho {
	struct RendererDate {
		std::shared_ptr<Shader> shader;
	};

	static RendererDate s_Data;


	void Renderer::Init(std::shared_ptr<Shader>& shader) {
		s_Data.shader = shader;
	}

	void Renderer::BeginScene(std::shared_ptr<Camera>& camera, const glm::mat4& transform) {
		glm::mat4 viewMat = camera->GetProjection() * glm::inverse(transform);
		glm::vec3 translation(viewMat[3][0], viewMat[3][1], viewMat[3][2]);
		s_Data.shader->Bind();
		s_Data.shader->SetVec3("u_ViewPosition", translation);
		s_Data.shader->SetMat4("u_Model", glm::mat4(1.0f));
	}

	// Temporary use for debugging
	void Renderer::BeginScene(std::shared_ptr<Camera>& camera) {
		s_Data.shader->Bind();
		s_Data.shader->SetMat4("u_ViewProjection", camera->GetProjection());
		s_Data.shader->SetVec3("u_ViewPosition", camera->GetPosition());
		s_Data.shader->SetVec3("u_LightPosition", glm::vec3(0.0f, 0.0, 0.0f));
		s_Data.shader->SetVec3("u_Color", glm::vec3(1.0f));
		s_Data.shader->SetVec3("u_LightColor", glm::vec3(1.0f));
		s_Data.shader->SetMat4("u_Model", glm::mat4(1.0f));
	}

	void Renderer::EndScene() {
		s_Data.shader->Unbind();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray) {
		s_Data.shader->Bind();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray) {
		shader->Bind();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}