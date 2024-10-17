#pragma once

#include "Runtime/Function/Renderer/RenderPass/RenderPass.h"
#include <vector>

namespace Aho {
	class RenderPipeline {
	public:
		~RenderPipeline();
		virtual void Initialize() = 0;
		virtual void Execute();
		std::shared_ptr<Framebuffer> GetRenderPass(size_t index);
		std::shared_ptr<Framebuffer> GetResultPass() { return m_ResultPass->GetRenderTarget(); }
		std::shared_ptr<Framebuffer> GetDepthPass() { return m_DepthPass->GetRenderTarget(); }
		std::shared_ptr<Framebuffer> GetDebugPass() { return m_DebugPass->GetRenderTarget(); }
		std::shared_ptr<Framebuffer> GetPickingPass() { return m_PickingPass->GetRenderTarget(); }
		virtual void SetRenderData(const std::vector<std::shared_ptr<RenderData>>& renderData) { m_RenderData = renderData; }
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { m_RenderData.push_back(data); }
		virtual void AddVirtualRenderData(const std::shared_ptr<RenderData>& data) { m_VirtualData.push_back(data); }
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) { m_RenderData.insert(m_RenderData.end(), data.begin(), data.end()); }
		virtual void AddRenderPass(RenderPass* rp) { m_RenderPasses.push_back(rp); SortRenderPasses(); }
		virtual void SortRenderPasses();
		std::vector<RenderPass*>::iterator begin() { return m_RenderPasses.begin(); }
		std::vector<RenderPass*>::iterator end() { return m_RenderPasses.end(); }
	protected:
		bool m_DrawDebug{ true };
		RenderPass* m_ResultPass{ nullptr };
		RenderPass* m_DebugPass{ nullptr };
		RenderPass* m_DepthPass{ nullptr };
		RenderPass* m_PickingPass{ nullptr };
		std::vector<RenderPass*> m_RenderPasses;	
		std::vector<std::shared_ptr<RenderData>> m_RenderData;	// render data is a per mesh basis
		std::vector<std::shared_ptr<RenderData>> m_VirtualData; // Such as light source, only renderred in picking pass
		std::vector<std::shared_ptr<RenderData>> m_DebugData;	// TODO: Temporary
	};


	// default forward pipeline
	class RenderPipelineDefault : public RenderPipeline {
	public:
		RenderPipelineDefault() {
			Initialize();
		}
		virtual void Initialize() override {
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
			std::shared_ptr<MeshInfo> meshInfo = std::make_shared<MeshInfo>(quadVertices, quadIndices, false, true);
			std::shared_ptr<VertexArray> quadVAO;
			quadVAO.reset(VertexArray::Create());
			quadVAO->Init(meshInfo);
			m_DebugData.push_back(std::make_shared<RenderData>(quadVAO));
			AHO_CORE_INFO("RenderPipelineDefault initialized");
		}
	};
} // namespace Aho
