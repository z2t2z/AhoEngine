#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPassBase.h"
#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

#include <glm/glm.hpp>

namespace Aho {
	class DispatchIndirectBuffer;
	struct alignas(16) Payload {
		glm::vec3 origin;
		bool alive; //4 bytes

		glm::vec3 direction;
		uint32_t bounce;

		glm::vec3 throughput;
		uint32_t pixelIndex;

		glm::vec3 radiance;
		float pdf;

		glm::vec3 N; // normal
		float cosTheta;

		glm::vec3 pos; // hit pos through interpolation, accurate than origin+direction*hitT
		float eta;
	};

	class EntityManager;
	class Shader;
	class PathTracingPipeline : public RenderPipeline {
	public:
		PathTracingPipeline();
		~PathTracingPipeline() = default;
		virtual void Initialize() override;
		virtual void Execute() override;
		virtual bool Resize(uint32_t width, uint32_t height) const override;
	public:
		void Init();
		void _Execute();
		bool UpdateSceneSSBOData() const;
	private:
		std::shared_ptr<DispatchIndirectBuffer> m_DispatchBuffer;
		std::unique_ptr<RenderPassBase> m_CameraRayGenPass;
		std::unique_ptr<RenderPassBase> m_IntersectionPass;
		std::unique_ptr<RenderPassBase> m_DispatchPrepPass;
		Shader* m_IntersectShader{ nullptr };
		uint32_t m_WriteIndex{ 1 };
		uint32_t m_ReadIndex{ 0 };
		_Texture* m_AccumulateTex{ nullptr };
		_Texture* m_PresentTex{ nullptr };
	private:
		bool SyncSceneDirtyFlags(const std::shared_ptr<EntityManager>& ecs) const;
		bool SyncActiveIBLLighting(const std::shared_ptr<EntityManager>& ecs, const Shader* shader) const;
	private:
		std::unique_ptr<RenderPassBase> m_AccumulatePass;
		std::unique_ptr<RenderPassBase> m_PresentPass;
	private:
		uint32_t m_CurrBounce{ 0 };
		uint32_t m_Frame{ 1 };
		uint32_t m_MaxBounce{ 8 };
	};
}