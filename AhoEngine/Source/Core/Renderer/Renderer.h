#pragma once

#include <memory>

#include <glm/glm.hpp>
#include "RenderCommand.h"
#include "Shader.h"
#include "Core/Camera/Camera.h"

namespace Aho {

	class Renderer {
	public:
		static void Init(std::shared_ptr<Shader>& shader);
		static void BeginScene(std::shared_ptr<Camera>& camera, const glm::mat4& transform);
		static void BeginScene(std::shared_ptr<Camera>& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
			std::shared_ptr<Shader> shader;
		};

		static std::unique_ptr<SceneData> s_SceneData;
		
	};

}

