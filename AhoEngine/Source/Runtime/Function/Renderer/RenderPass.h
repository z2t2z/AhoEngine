#pragma once
#include "RHI.h"
#include "RenderCommand.h"
#include "RenderData.h"
#include "Framebuffer.h"

namespace Aho {
	class RenderPass {
	public:
		~RenderPass() = default;
		virtual void Initialize() = 0;
		virtual void SetRenderTarget(const std::shared_ptr<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		virtual void SetRenderData(const std::vector<std::shared_ptr<RenderData>>& renderData) { m_RenderData = renderData; }
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { m_RenderData.push_back(data); }
		virtual void SetRenderCommand(RenderCommandBuffer* renderCommandBuffer) { m_RenderCommandBuffer = renderCommandBuffer; }
		virtual void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		virtual std::shared_ptr<Shader> GetShader() { return m_Shader; }
		virtual RenderCommandBuffer* GetRenderCommandBuffer() { return m_RenderCommandBuffer; }
		virtual void Execute() = 0;
		std::shared_ptr<Framebuffer> GetRenderTarget() { return m_Framebuffer; }
	protected:
		//std::shared_ptr<RHI> m_Rhi; TODO
		std::shared_ptr<Shader> m_Shader{ nullptr };
		std::vector<std::shared_ptr<RenderData>> m_RenderData;
		std::shared_ptr<Framebuffer> m_Framebuffer{ nullptr };
		RenderCommandBuffer* m_RenderCommandBuffer{ nullptr };
	};

	class RenderPassForward : public RenderPass {
	public:
		RenderPassForward() { Initialize(); }
		~RenderPassForward() = default;
		virtual void Initialize() override {
			AHO_CORE_INFO("Forward render pass initialized");
		}
		virtual void Execute() override {
			if (!m_Framebuffer) {
				AHO_CORE_WARN("Render target is not initialized!");
				return;
			}
			if (m_RenderData.empty()) {
				//AHO_CORE_WARN("No data to render!");
				return;
			}
			//m_Shader->Bind();
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));
			RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
			for (const auto& data : m_RenderData) {
				m_RenderCommandBuffer->Execute(data);
			}
			m_Framebuffer->Unbind();
			//m_Shader->Unbind();
		}
	};

	class RenderPassDeferred : public RenderPass {
		RenderPassDeferred() { Initialize(); }
		~RenderPassDeferred() = default;
		virtual void Initialize() override {
			AHO_CORE_INFO("Forward render pass initialized");
		}
		virtual void Execute() override {
			if (!m_Framebuffer) {
				AHO_CORE_WARN("Render target is not initialized!");
				return;
			}
			if (m_RenderData.empty()) {
				AHO_CORE_WARN("No data to render!");
				return;
			}
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));
			RenderCommand::Clear(ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer);
			for (const auto& data : m_RenderData) {
				m_RenderCommandBuffer->Execute(data);
			}
			m_Framebuffer->Unbind();
		}
	};
}
