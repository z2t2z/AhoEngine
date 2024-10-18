#pragma once
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Framebuffer.h"
#include <vector>

namespace Aho {
	enum class RenderPassType {
		None = 0,
		Debug,
		Final,
		Depth,
		Pick,
		SSAOGeo,
		SSAO,
		/* TODO */
	};

	class RenderPass {
	public:
		~RenderPass() = default;
		virtual void Initialize() = 0;
		virtual void SetRenderTarget(const std::shared_ptr<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		virtual void SetRenderCommand(RenderCommandBuffer* renderCommandBuffer) { m_RenderCommandBuffer = renderCommandBuffer; }
		virtual void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		virtual RenderPassType GetRenderPassType() { return m_RenderPassType; }
		virtual void SetRenderPassType(RenderPassType type) { m_RenderPassType = type; }
		virtual const std::shared_ptr<Framebuffer>& Execute(const std::vector<std::shared_ptr<RenderData>>& renderData) = 0;
		virtual void BindSceneDataUBO(const UBO& m_UBO) { m_Shader->BindUBO(m_UBO); }
		virtual void AddGBuffer(Texture* tex) { m_TextureBuffers.push_back(tex); }
		virtual std::shared_ptr<Shader> GetShader() { return m_Shader; }
		virtual RenderCommandBuffer* GetRenderCommandBuffer() { return m_RenderCommandBuffer; }
		std::shared_ptr<Framebuffer> GetRenderTarget() { return m_Framebuffer; }
	protected:
		std::shared_ptr<Framebuffer> m_Framebuffer{ nullptr };  // This is the render target of this pass
	protected:
		std::vector<Texture*> m_TextureBuffers; // G-buffers are stored here 
		RenderPassType m_RenderPassType{ RenderPassType::None };
		std::shared_ptr<Shader> m_Shader{ nullptr };   // Currently each render pass is completed using a single shader
		RenderCommandBuffer* m_RenderCommandBuffer{ nullptr };
	};

	class RenderPassDefault : public RenderPass {
	public:
		RenderPassDefault() { Initialize(); }
		~RenderPassDefault() = default;
		virtual void Initialize() override {
			AHO_CORE_INFO("Forward render pass initialized");
		}
		virtual const std::shared_ptr<Framebuffer>& Execute(const std::vector<std::shared_ptr<RenderData>>& renderData) override {
			m_RenderCommandBuffer->Execute(renderData, m_Shader, m_Framebuffer, m_TextureBuffers);
			return m_Framebuffer;
		}
	};
};