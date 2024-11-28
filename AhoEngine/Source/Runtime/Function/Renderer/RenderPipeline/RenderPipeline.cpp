#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPassTarget(RenderPassType type) {
		auto it = std::find_if(m_RenderTasks.begin(), m_RenderTasks.end(), [type](const RenderTask& task) {
			return task.pass->GetRenderPassType() == type;
		});		
		return it != m_RenderTasks.end() ? it->pass->GetRenderTarget() : nullptr;
	}

	const std::vector<std::shared_ptr<RenderData>>& RenderPipeline::GetRenderData(RenderDataType type) {
		switch (type) {
			case RenderDataType::SceneData:
				return m_SceneData;
			case RenderDataType::ScreenQuad:
				return m_ScreenQuad;
			case RenderDataType::DebugData:
				return m_DebugData;
			case RenderDataType::UnitCube:
				return m_UnitCube;
		}
		AHO_CORE_ERROR("wrong render data type");
	}

	void RenderPipeline::ResizeRenderTarget(uint32_t width, uint32_t height) {
		for (auto& task : m_RenderTasks) {
			if (task.pass->GetRenderPassType() != RenderPassType::Depth) {
				task.pass->GetRenderTarget()->Resize(width, height);
			}
		}
	}

	void RenderPipeline::Initialize() {
		// Screen Quad
		Vertex upperLeft, lowerLeft, upperRight, lowerRight;
		upperLeft.position = glm::vec3(-1.0, 1.0, 0.0);
		upperLeft.uv = glm::vec2(0.0, 1.0);

		lowerLeft.position = glm::vec3(-1.0, -1.0, 0.0);
		lowerLeft.uv = glm::vec2(0.0, 0.0);

		lowerRight.position = glm::vec3(1.0, -1.0, 0.0);
		lowerRight.uv = glm::vec2(1.0, 0.0);
		
		upperRight.position = glm::vec3(1.0, 1.0, 0.0);
		upperRight.uv = glm::vec2(1.0, 1.0);

		std::vector<Vertex> quadVertices = { upperLeft, lowerLeft, lowerRight, upperRight };
		std::vector<uint32_t> quadIndices = {
			0, 1, 2,
			2, 3, 0
		};
		{
			std::shared_ptr<MeshInfo> meshInfo = std::make_shared<MeshInfo>(quadVertices, quadIndices, false, true);
			std::shared_ptr<VertexArray> quadVAO;
			quadVAO.reset(VertexArray::Create());
			quadVAO->Init(meshInfo);
			m_ScreenQuad.push_back(std::make_shared<RenderData>(quadVAO));
		}

		// Unit cube
		Vertex frontUpperLeft, frontLowerLeft, frontUpperRight, frontLowerRight;
		Vertex backUpperLeft, backLowerLeft, backUpperRight, backLowerRight;

		// Front face
		frontUpperLeft.position = glm::vec3(-1.0, 1.0, 1.0);
		frontUpperLeft.uv = glm::vec2(0.0, 1.0);

		frontLowerLeft.position = glm::vec3(-1.0, -1.0, 1.0);
		frontLowerLeft.uv = glm::vec2(0.0, 0.0);
		
		frontUpperRight.position = glm::vec3(1.0, 1.0, 1.0);
		frontUpperRight.uv = glm::vec2(1.0, 1.0);

		frontLowerRight.position = glm::vec3(1.0, -1.0, 1.0);
		frontLowerRight.uv = glm::vec2(1.0, 0.0);

		// Back face
		backUpperLeft.position = glm::vec3(-1.0, 1.0, -1.0);
		backUpperLeft.uv = glm::vec2(1.0, 1.0);

		backLowerLeft.position = glm::vec3(-1.0, -1.0, -1.0);
		backLowerLeft.uv = glm::vec2(1.0, 0.0);

		backUpperRight.position = glm::vec3(1.0, 1.0, -1.0);
		backUpperRight.uv = glm::vec2(0.0, 1.0);

		backLowerRight.position = glm::vec3(1.0, -1.0, -1.0);
		backLowerRight.uv = glm::vec2(0.0, 0.0);

		std::vector<Vertex> cubeVertices = {
			frontUpperLeft, frontLowerLeft, frontUpperRight, frontLowerRight,
			backUpperLeft, backLowerLeft, backUpperRight, backLowerRight
		};

		std::vector<uint32_t> cubeIndices = {
			// Front face
			0, 1, 2, 1, 3, 2,
			// Back face
			4, 5, 6, 5, 7, 6,
			// Left face
			0, 1, 4, 1, 5, 4,
			// Right face
			2, 3, 6, 3, 7, 6,
			// Top face
			0, 2, 4, 2, 6, 4,
			// Bottom face
			1, 3, 5, 3, 7, 5
		};
		{
			std::shared_ptr<MeshInfo> meshInfo = std::make_shared<MeshInfo>(cubeVertices, cubeIndices, false, true);
			std::shared_ptr<VertexArray> cubeVAO;
			cubeVAO.reset(VertexArray::Create());
			cubeVAO->Init(meshInfo);
			m_UnitCube.push_back(std::make_shared<RenderData>(cubeVAO));
		}
	}

	void RenderPipeline::Execute() {
		for (const auto& task : m_RenderTasks) {
			task.pass->Execute(GetRenderData(task.dataType));
		}
	}
} // namespace Aho