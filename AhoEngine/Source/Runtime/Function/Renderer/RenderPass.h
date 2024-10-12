#pragma once
#include "RHI.h"
#include "RenderCommand.h"
#include "RenderData.h"
#include "Framebuffer.h"
#include <vector>

namespace Aho {
	class RenderPass {
	public:
		~RenderPass() = default;
		virtual void Initialize() = 0;
		virtual void SetRenderTarget(const std::shared_ptr<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		virtual void SetRenderData(const std::vector<std::shared_ptr<RenderData>>& renderData) { m_RenderData = renderData; }
		virtual void AddRenderData(const std::shared_ptr<RenderData>& data) { m_RenderData.push_back(data); }
		virtual void AddRenderData(const std::vector<std::shared_ptr<RenderData>>& data) { m_RenderData.insert(m_RenderData.end(), data.begin(), data.end()); }
		virtual void SetRenderCommand(RenderCommandBuffer* renderCommandBuffer) { m_RenderCommandBuffer = renderCommandBuffer; }
		virtual void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		virtual void SetClearColor(glm::vec4 color) { m_ClearColor = color; }
		virtual void SetClearFlags(ClearFlags flags) { m_ClearFlags = flags; }
		virtual std::shared_ptr<Shader> GetShader() { return m_Shader; }
		virtual RenderCommandBuffer* GetRenderCommandBuffer() { return m_RenderCommandBuffer; }
		virtual void Execute() = 0;
		std::shared_ptr<Framebuffer> GetRenderTarget() { return m_Framebuffer; }
	protected:
		//std::shared_ptr<RHI> m_Rhi; TODO
		glm::vec4 m_ClearColor{1.0f, 1.0f, 1.0f, 1.0f};
		ClearFlags m_ClearFlags{ ClearFlags::Color_Buffer | ClearFlags::Depth_Buffer };
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
			m_Shader->Bind();
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor(m_ClearColor);
			RenderCommand::Clear(m_ClearFlags);
			for (const auto& data : m_RenderData) {
				m_RenderCommandBuffer->Execute(data);
			}
			m_Framebuffer->Unbind();
			m_Shader->Unbind();
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
			RenderCommand::SetClearColor(m_ClearColor);
			RenderCommand::Clear(m_ClearFlags);
			for (const auto& data : m_RenderData) {
				m_RenderCommandBuffer->Execute(data);
			}
			m_Framebuffer->Unbind();
		}
	};
}
