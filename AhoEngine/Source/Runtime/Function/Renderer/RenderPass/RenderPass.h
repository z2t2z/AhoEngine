#pragma once
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/Framebuffer.h"
#include <vector>

namespace Aho {
	enum class RenderPassType {
		None = 0,
		Debug,
		Shading,
		Depth,
		Pick,
		SSAOGeo,
		SSAO,
		SSAOLighting,
		BlurR,
		BlurRGB,
		SSRvs,
		HiZ,
		DrawLine,
		PostProcessing,
		PrecomputeIrradiance,
		GenCubemap,
		Prefilter,
		GenLUT
		/* TODO */
	};

	struct TextureBuffers {
		Texture* tex{ nullptr };
		std::string name;
	};

	class RenderPass {
	public:
		~RenderPass() = default;
		virtual void SetRenderTarget(const std::shared_ptr<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		virtual void SetRenderCommand(std::unique_ptr<RenderCommandBuffer> renderCommandBuffer) { m_RenderCommandBuffer = std::move(renderCommandBuffer); }
		virtual void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		virtual RenderPassType GetRenderPassType() { return m_RenderPassType; }
		virtual void SetRenderPassType(RenderPassType type) { m_RenderPassType = type; }
		
		virtual void Execute(const std::vector<std::shared_ptr<RenderData>>& renderData) {
			m_RenderCommandBuffer->Execute(renderData, m_Shader, m_Framebuffer, m_TextureBuffers);
		}

		virtual void RegisterTextureBuffer(const TextureBuffers& buffer) { m_TextureBuffers.push_back(buffer); }

		virtual std::shared_ptr<Shader> GetShader() { return m_Shader; }
		std::shared_ptr<Framebuffer> GetRenderTarget() { return m_Framebuffer; }

	private:
		std::vector<TextureBuffers> m_TextureBuffers;
		std::shared_ptr<Framebuffer> m_Framebuffer{ nullptr };  // This is the render target of this pass
		RenderPassType m_RenderPassType{ RenderPassType::None };
		std::shared_ptr<Shader> m_Shader{ nullptr };			// Currently each render pass uses a single shader
		std::unique_ptr<RenderCommandBuffer> m_RenderCommandBuffer{ nullptr };
	};

};