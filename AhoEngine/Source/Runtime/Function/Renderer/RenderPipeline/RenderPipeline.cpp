#include "Ahopch.h"
#include "RenderPipeline.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
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

	std::shared_ptr<Framebuffer> RenderPipeline::GetRenderPassTarget(RenderPassType type) {
		auto it = std::find_if(m_RenderTasks.begin(), m_RenderTasks.end(), 
			[type](const RenderTask& task) {
				return task.pass->GetRenderPassType() == type;
			});		
		return it != m_RenderTasks.end() ? it->pass->GetRenderTarget() : nullptr;
	}

	// Delete these
	void RenderTask::Init() {
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