#pragma once

#include "Runtime/Function/Renderer/RenderPipeline/RenderPipeline.h"

namespace Aho {
	class PathTracingPipeline : public RenderPipeline {
	public:
		PathTracingPipeline();
		virtual void Initialize() override;
		virtual void Execute() override;

		void SetBVH(BVHNode* root) {
			m_Root = root;
		}

		void OnResize(uint32_t width, uint32_t height);

	private:

	private:
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