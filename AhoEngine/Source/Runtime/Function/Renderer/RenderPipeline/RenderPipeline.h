#pragma once

#include <vector>
#include <memory>

namespace Aho {
	class RenderPass;
	class RenderData;
	class Framebuffer;
	class Texture;
	enum class RenderPassType;

	enum class RenderDataType {
		Empty = 0,
		SceneData,
		ScreenQuad,
		DebugData,
		UnitCube,
		UnitCircle
	};
	enum class RenderPipelineType {
		None = 0,
		RPL_PathTracing,
		RPL_Default,
		RPL_RenderSky,
		RPL_IBL,
		RPL_PostProcess,
		RPL_DeferredShading,
		RPL_DebugVisual,
	};

	struct RenderTask {
		RenderPass* pass;
		RenderDataType dataType;

		static void Init();
		static const std::vector<std::shared_ptr<RenderData>>& GetRenderData(RenderDataType type);
		static std::vector<std::shared_ptr<RenderData>> m_ScreenQuad;
		static std::vector<std::shared_ptr<RenderData>> m_UnitCube;
		static std::vector<std::shared_ptr<RenderData>> m_SceneData;	
		static std::vector<std::shared_ptr<RenderData>> m_DebugData;
		static std::vector<std::shared_ptr<RenderData>> m_EmptyVao;
		static std::vector<std::shared_ptr<RenderData>> m_UnitCircle;
	};

	class _Texture;
	class RenderPassBase;
	class RenderPipeline {
	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;
		virtual void Initialize() = 0;
		virtual void Execute();
		virtual RenderPass* GetRenderPass(RenderPassType type);
		virtual RenderPipelineType GetType() { return m_Type; }
		virtual Texture* GetRenderResult() { return m_RenderResult; }
		virtual uint32_t GetRenderResultTextureID() const;
		virtual void SetRenderTarget(RenderPassType type, const std::shared_ptr<Framebuffer>& fbo);
		virtual bool ResizeRenderTarget(uint32_t width, uint32_t height);
		virtual bool Resize(uint32_t width, uint32_t height) const { return false; }
		virtual void RegisterRenderPass(RenderPass* renderPass, RenderDataType type) {
			m_RenderTasks.emplace_back(renderPass, type);
		}
		virtual std::shared_ptr<Framebuffer> GetRenderPassTarget(RenderPassType type);
		virtual void SetInput(Texture* tex) { m_Input = tex; }
		virtual _Texture* GetTextureBufferByIndex(size_t idx) const {
			if (idx < m_TextureBuffers.size()) {
				return m_TextureBuffers[idx].get();
			}
			return nullptr;
		}
		virtual std::vector<std::shared_ptr<_Texture>> GetBuffers() const { return m_TextureBuffers; }
	protected:
		uint32_t m_ResultTextureID{ 0 };
		_Texture* m_Result{ nullptr };
	protected:
		RenderPipelineType m_Type = RenderPipelineType::RPL_Default;

	//Delete these
	protected:
		std::vector<RenderTask> m_RenderTasks;
		Texture* m_RenderResult{ nullptr };
		Texture* m_Input{ nullptr };
		std::vector<std::shared_ptr<_Texture>> m_TextureBuffers;

	};

} // namespace Aho
