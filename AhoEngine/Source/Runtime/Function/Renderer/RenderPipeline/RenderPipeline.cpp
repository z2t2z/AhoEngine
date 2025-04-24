#include "Ahopch.h"
#include "RenderPipeline.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"

namespace Aho {
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_ScreenQuad;
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_UnitCube;
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_SceneData;	// render data is a per mesh basis
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_DebugData;
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_EmptyVao;
	std::vector<std::shared_ptr<RenderData>> RenderTask::m_UnitCircle; // line

	void RenderPipeline::Execute() {
		for (const auto& task : m_RenderTasks) {
			RenderCommand::PushDebugGroup(task.pass->GetPassDebugName());
			task.pass->Execute(RenderTask::GetRenderData(task.dataType));
			RenderCommand::PopDebugGroup();
		}
	}

	RenderPass* RenderPipeline::GetRenderPass(RenderPassType type) {
		for (const auto& task : m_RenderTasks) {
			if (task.pass->GetRenderPassType() == type) {
				return task.pass;
			}
		}
		return nullptr;
	}

	void RenderPipeline::SetRenderTarget(RenderPassType type, const std::shared_ptr<Framebuffer>& fbo) {
		auto pass = GetRenderPass(type);
		pass->SetRenderTarget(fbo);
		m_RenderResult = pass->GetTextureBuffer(TexType::Result);
	}

	bool RenderPipeline::ResizeRenderTarget(uint32_t width, uint32_t height) {
		bool resized = false;
		for (auto& task : m_RenderTasks) {
			resized |= task.pass->GetRenderTarget()->Resize(width, height);
		}
		return resized;
	}

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPassTarget(RenderPassType type) {
		auto it = std::find_if(m_RenderTasks.begin(), m_RenderTasks.end(), 
			[type](const RenderTask& task) {
				return task.pass->GetRenderPassType() == type;
			});		
		return it != m_RenderTasks.end() ? it->pass->GetRenderTarget() : nullptr;
	}

	void RenderTask::Init() {
		// Empty vao, maybe replace quad vao with this for performance?
		{
			std::shared_ptr<VertexArray> quadVAO;
			quadVAO.reset(VertexArray::Create());
			m_EmptyVao.push_back(std::make_shared<RenderData>(quadVAO));
		}

		// Screen Quad
		Vertex upperLeft, lowerLeft, upperRight, lowerRight;
		upperLeft.position = glm::vec3(-1.0, 1.0, 0.0);
		upperLeft.u = glm::vec2(0.0, 1.0).x;
		upperLeft.v = glm::vec2(0.0, 1.0).y;


		lowerLeft.position = glm::vec3(-1.0, -1.0, 0.0);
		lowerLeft.u = glm::vec2(0.0, 0.0).x;
		lowerLeft.v = glm::vec2(0.0, 0.0).y;

		lowerRight.position = glm::vec3(1.0, -1.0, 0.0);
		lowerRight.u = glm::vec2(1.0, 0.0).x;
		lowerRight.v = glm::vec2(1.0, 0.0).y;

		upperRight.position = glm::vec3(1.0, 1.0, 0.0);
		upperRight.u = glm::vec2(1.0, 1.0).x;
		upperRight.v = glm::vec2(1.0, 1.0).y;


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
		frontUpperLeft.u = 0.0f;
		frontUpperLeft.v = 1.0f;

		frontLowerLeft.position = glm::vec3(-1.0, -1.0, 1.0);
		frontLowerLeft.u = 0.0f;
		frontLowerLeft.v = 0.0f;

		frontUpperRight.position = glm::vec3(1.0, 1.0, 1.0);
		frontUpperRight.u = 1.0f;
		frontUpperRight.v = 1.0f;

		frontLowerRight.position = glm::vec3(1.0, -1.0, 1.0);
		frontLowerRight.u = 1.0f;
		frontLowerRight.v = 0.0f;

		// Back face
		backUpperLeft.position = glm::vec3(-1.0, 1.0, -1.0);
		backUpperLeft.u = 1.0f;
		backUpperLeft.v = 1.0f;

		backLowerLeft.position = glm::vec3(-1.0, -1.0, -1.0);
		backLowerLeft.u = 1.0f;
		backLowerLeft.v = 0.0f;

		backUpperRight.position = glm::vec3(1.0, 1.0, -1.0);
		backUpperRight.u = 0.0f;
		backUpperRight.v = 1.0f;

		backLowerRight.position = glm::vec3(1.0, -1.0, -1.0);
		backLowerRight.u = 0.0f;
		backLowerRight.v = 0.0f;

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


		// Unit circle
		{
			int numSegments = 32;
			std::vector<float> vertices;
			std::vector<uint32_t> indices;
			for (int i = 0; i < numSegments; i++) {
				float theta = 2.0f * glm::pi<float>() * i / numSegments;
				float x = cos(theta);
				float y = sin(theta);
				vertices.push_back(x);
				vertices.push_back(0.0f);
				vertices.push_back(y);

				indices.push_back(i);
				indices.push_back((i + 1) % numSegments); 
			}

			std::shared_ptr<LineInfo> lineInfo = std::make_shared<LineInfo>(vertices, indices);
			std::shared_ptr<VertexArray> circleVAO;
			circleVAO.reset(VertexArray::Create());
			circleVAO->Init(lineInfo);
			m_UnitCircle.push_back(std::make_shared<RenderData>(circleVAO));
		}
	}

	const std::vector<std::shared_ptr<RenderData>>& RenderTask::GetRenderData(RenderDataType type) {
		switch (type) {
			case RenderDataType::Empty:
				return m_EmptyVao;
			case RenderDataType::SceneData:
				return m_SceneData;
			case RenderDataType::ScreenQuad:
				return m_ScreenQuad;
			case RenderDataType::DebugData:
				return m_DebugData;
			case RenderDataType::UnitCube:
				return m_UnitCube;
			case RenderDataType::UnitCircle:
				return m_UnitCircle;
		}
		AHO_CORE_ERROR("wrong render data type");
	}

} // namespace Aho