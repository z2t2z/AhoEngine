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

	void Renderer::BeginScene(const Camera* camera, const glm::mat4& transform) {
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		auto result = rotation * transform;
		auto proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
		//glm::mat4 viewMat = camera->GetProjection() * glm::inverse(transform);
		glm::mat4 viewMat = proj * glm::inverse(result);
		s_Data.shader->Bind();
		s_Data.shader->SetMat4("u_ViewProjection", viewMat);
		s_Data.shader->SetMat4("u_Model", glm::mat4(1.0f));
		//auto test_color = glm::vec4(transform[0][3], transform[1][3], transform[2][3], 1.0f);
		//s_Data.shader->SetVec4("u_color", test_color);
		s_Data.shader->SetVec4("u_color", glm::vec4(1.0, 0.0, 0.0, 1.0));
	}

	void Renderer::BeginScene(const Camera* camera, const glm::mat4& transform, glm::vec4& color) {
		glm::mat4 viewMat = camera->GetProjection() * glm::inverse(transform);
		s_Data.shader->Bind();
		s_Data.shader->SetMat4("u_ViewProjection", viewMat);
		s_Data.shader->SetMat4("u_Model", glm::mat4(1.0f));
		s_Data.shader->SetVec4("u_color", color);
	}

	void Renderer::EndScene() {

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