#pragma once
#include "RHI.h"

namespace Aho {
	class RenderResource;

	class RenderPass {
	public:
		virtual void Initialize() = 0;
		virtual void SetRenderResource(std::shared_ptr<RenderResource> m_RenderResource) = 0;
	protected:
		std::shared_ptr<RHI> m_Rhi;
		std::shared_ptr<RenderResource> m_RenderResource;
	};
}
