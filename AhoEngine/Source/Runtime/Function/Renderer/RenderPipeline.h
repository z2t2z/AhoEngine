#pragma once

#include "RenderPass.h"
#include <vector>

namespace Aho {
	class RenderPipeline {
	public:
		~RenderPipeline() {
			for (auto renderPass : m_RenderPasses) {
				delete renderPass;
			}
		}
		virtual void Initialize() = 0;
		virtual void Execute() const {
			std::shared_ptr<Framebuffer> fbo{ nullptr };   // Seems like a bad way but can't tell the reason
			for (const auto& renderPass : m_RenderPasses) {
				if (renderPass->GetRenderPassType() != RenderPassType::Debug) {
					fbo = renderPass->Execute(m_RenderData, fbo);
				}
			}
			if (m_DrawDebug && fbo) {
				for (const auto& renderPass : m_RenderPasses) {
					if (renderPass->GetRenderPassType() == RenderPassType::Debug) {
						fbo = renderPass->Execute(m_DebugData, fbo);
					}
				}
			}
		}
		RenderPass* GetRenderPass(size_t index) {
			if (index >= m_RenderPasses.size()) {
				AHO_CORE_ERROR("Out of bound in render pass at index {}", index);
				return nullptr;
			}
			return m_RenderPasses[index];
		}
		virtual void SetRenderData(const std::vector<std::shared_ptr<RenderData>>& renderData) { m_RenderData = renderData; }
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { m_RenderData.push_back(data); }
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) { m_RenderData.insert(m_RenderData.end(), data.begin(), data.end()); }
		virtual void AddRenderPass(RenderPass* rp) { m_RenderPasses.push_back(rp); }
		std::vector<RenderPass*>::iterator begin() { return m_RenderPasses.begin(); }
		std::vector<RenderPass*>::iterator end() { return m_RenderPasses.end(); }
	protected:
		bool m_DrawDebug{ true };
		std::vector<std::shared_ptr<RenderData>> m_RenderData;	// render data is a per mesh basis
		std::vector<RenderPass*> m_RenderPasses;	
		std::vector<std::shared_ptr<RenderData>> m_DebugData;	// TODO: Temporary
		std::vector<RenderPass*> m_DebugPasses;
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
