#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class PathTracerPipeline : public RenderPipeline {
	public:
		PathTracerPipeline();
		virtual void Initialize() override {}
		virtual void Execute() override;

		void SetBVH(BVHNode* root) {
			m_Root = root;
		}
		void SetCamera(const std::shared_ptr<Camera>& cam) {
			m_Camera = cam;
		}

		void OnResize(uint32_t width, uint32_t height);

	private:
		glm::vec4 SampleRadiance(IntersectResult result);
		glm::vec4 PerPixelShading(uint32_t x, uint32_t y);
	private:
		std::shared_ptr<Camera> m_Camera;
		std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter; // iterators for multi-threading
		BVHNode* m_Root;
		uint32_t m_FrameIndex = 1;
		bool m_Accumulate{ false };
		uint32_t* m_ImageData = nullptr;
		glm::vec4* m_AccumulationData = nullptr;
	private:
		TexSpec m_TexSpec;
		std::shared_ptr<Texture2D> m_FinalImage;
		std::shared_ptr<Texture2D> m_Noise;
	};

}