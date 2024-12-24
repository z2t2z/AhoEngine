#pragma once

#include "Runtime/Core/BVH.h"
#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include <vector>

namespace Aho {
	enum class RenderDataType {
		Empty = 0,
		SceneData,
		ScreenQuad,
		DebugData,
		UnitCube
	};

	enum class RenderPipelineType {
		None = 0,
		RPL_PathTracing,
		RPL_Default,
		RPL_RenderSky,
		RPL_IBL,
		RPL_PostProcess,
		RPL_DeferredShading
	};

	struct RenderTask {
		RenderPass* pass;
		RenderDataType dataType;

		static void Init();
		static const std::vector<std::shared_ptr<RenderData>>& GetRenderData(RenderDataType type);
		static std::vector<std::shared_ptr<RenderData>> m_ScreenQuad;
		static std::vector<std::shared_ptr<RenderData>> m_UnitCube;
		static std::vector<std::shared_ptr<RenderData>> m_SceneData;	// render data is a per mesh basis
		static std::vector<std::shared_ptr<RenderData>> m_DebugData;
		static std::vector<std::shared_ptr<RenderData>> m_EmptyVao;

	};

	class RenderPipeline {
	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;
		virtual void Initialize() = 0;

		virtual void Execute() {
			for (const auto& task : m_RenderTasks) {
				RenderCommand::PushDebugGroup(task.pass->GetPassDebugName());
				task.pass->Execute(RenderTask::GetRenderData(task.dataType));
				RenderCommand::PopDebugGroup();
			}
		}

		virtual RenderPass* GetRenderPass(RenderPassType type) {
			for (auto& task : m_RenderTasks) {
				if (task.pass->GetRenderPassType() == type) {
					return task.pass;
				}
			}
			return nullptr;
		}

		virtual RenderPipelineType GetType() { return m_Type; }
		virtual Texture* GetRenderResult() { return m_RenderResult; }

		virtual bool ResizeRenderTarget(uint32_t width, uint32_t height) {
			bool resized = false;
			for (auto& task : m_RenderTasks) {
				resized |= task.pass->GetRenderTarget()->Resize(width, height);
			}
			return resized;
		}

		virtual void RegisterRenderPass(RenderPass* renderPass, RenderDataType type) {
			m_RenderTasks.emplace_back(renderPass, type);
		}
		virtual std::shared_ptr<Framebuffer> GetRenderPassTarget(RenderPassType type);
		virtual void SetInput(Texture* tex) { m_Input = tex; }

	protected:
		Texture* m_RenderResult{ nullptr };
		Texture* m_Input{ nullptr };

	protected:
		std::vector<RenderTask> m_RenderTasks;
		RenderPipelineType m_Type = RenderPipelineType::RPL_Default;

	};

} // namespace Aho
