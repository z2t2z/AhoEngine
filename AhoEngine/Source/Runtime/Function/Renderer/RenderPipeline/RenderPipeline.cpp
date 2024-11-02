#include "Ahopch.h"
#include "RenderPipeline.h"

namespace Aho {
	RenderPipeline::~RenderPipeline() {
		for (auto data : m_ScreenQuad) {
			delete data->GetTransformParam();
		}
		for (auto data : m_UnitCube) {
			delete data->GetTransformParam();
		}
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
		upperLeft.x = -1.0f, upperLeft.y = 1.0f, upperLeft.u = 0.0f, upperLeft.v = 1.0f;
		lowerLeft.x = -1.0f, lowerLeft.y = -1.0f, lowerLeft.u = 0.0f, lowerLeft.v = 0.0f;
		lowerRight.x = 1.0f, lowerRight.y = -1.0f, lowerRight.u = 1.0f, lowerRight.v = 0.0f;
		upperRight.x = 1.0f, upperRight.y = 1.0f, upperRight.u = 1.0f, upperRight.v = 1.0f;
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
			m_ScreenQuad.back()->SetTransformParam(new TransformParam());
		}

		// Unit cube
		Vertex frontUpperLeft, frontLowerLeft, frontUpperRight, frontLowerRight;
		Vertex backUpperLeft, backLowerLeft, backUpperRight, backLowerRight;

		// Front face
		frontUpperLeft.x = -1.0f, frontUpperLeft.y = 1.0f, frontUpperLeft.z = 1.0f, frontUpperLeft.u = 0.0f, frontUpperLeft.v = 1.0f;
		frontLowerLeft.x = -1.0f, frontLowerLeft.y = -1.0f, frontLowerLeft.z = 1.0f, frontLowerLeft.u = 0.0f, frontLowerLeft.v = 0.0f;
		frontUpperRight.x = 1.0f, frontUpperRight.y = 1.0f, frontUpperRight.z = 1.0f, frontUpperRight.u = 1.0f, frontUpperRight.v = 1.0f;
		frontLowerRight.x = 1.0f, frontLowerRight.y = -1.0f, frontLowerRight.z = 1.0f, frontLowerRight.u = 1.0f, frontLowerRight.v = 0.0f;

		// Back face
		backUpperLeft.x = -1.0f, backUpperLeft.y = 1.0f, backUpperLeft.z = -1.0f, backUpperLeft.u = 1.0f, backUpperLeft.v = 1.0f;
		backLowerLeft.x = -1.0f, backLowerLeft.y = -1.0f, backLowerLeft.z = -1.0f, backLowerLeft.u = 1.0f, backLowerLeft.v = 0.0f;
		backUpperRight.x = 1.0f, backUpperRight.y = 1.0f, backUpperRight.z = -1.0f, backUpperRight.u = 0.0f, backUpperRight.v = 1.0f;
		backLowerRight.x = 1.0f, backLowerRight.y = -1.0f, backLowerRight.z = -1.0f, backLowerRight.u = 0.0f, backLowerRight.v = 0.0f;

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
			m_UnitCube.back()->SetTransformParam(new TransformParam());
		}
	}

	void RenderPipeline::Execute() {
		for (const auto& task : m_RenderTasks) {
			task.pass->Execute(GetRenderData(task.dataType));
		}
	}
} // namespace Aho