#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include <vector>

namespace Aho {
	enum class RenderDataType {
		None = 0,
		SceneData,
		ScreenQuad,
		DebugData,
		UnitCube
	};

	enum class RenderPipelineType {
		None = 0,
		Precompute,
		Default
	};

	struct RenderTask {
		std::unique_ptr<RenderPass> pass;
		RenderDataType dataType;
	};
	

	class RenderPipeline {
	public:
		RenderPipeline() { Initialize(); }
		~RenderPipeline();
		virtual void Initialize();
		virtual void Execute();
		virtual void ResizeRenderTarget(uint32_t width, uint32_t height);
		virtual std::shared_ptr<Framebuffer> GetRenderPassTarget(RenderPassType type);
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { (data->IsDebug() ? m_DebugData : m_SceneData).push_back(data); }
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) { for (const auto& d : data) AddRenderData(d); }
		virtual void RegisterRenderPass(std::unique_ptr<RenderPass> renderPass, RenderDataType type) {
			m_RenderTasks.emplace_back(std::move(renderPass), type);
		}
		virtual const std::unique_ptr<RenderPass>& GetRenderPass(RenderPassType type) {
			for (auto& task : m_RenderTasks) {
				if (task.pass->GetRenderPassType() == type) {
					return task.pass;
				}
			}
			return nullptr;
		}
		virtual void SetType(RenderPipelineType type) { m_Type = type; }
		virtual RenderPipelineType GetType() { return m_Type; }
	private:
		RenderPipelineType m_Type = RenderPipelineType::Default;
		virtual const std::vector<std::shared_ptr<RenderData>>& GetRenderData(RenderDataType type);
	private:
		// TODO: consider decoupling renderdata 
		std::vector<std::shared_ptr<RenderData>> m_ScreenQuad;  
		std::vector<std::shared_ptr<RenderData>> m_UnitCube;
		std::vector<std::shared_ptr<RenderData>> m_SceneData;	// render data is a per mesh basis
		std::vector<std::shared_ptr<RenderData>> m_DebugData;
	private:
		std::vector<RenderTask> m_RenderTasks;
	};
} // namespace Aho
