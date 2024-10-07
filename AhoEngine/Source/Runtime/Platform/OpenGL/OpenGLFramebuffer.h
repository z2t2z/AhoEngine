#pragma once

#include "Runtime/Function/Renderer/FrameBuffer.h"

namespace Aho {
	class OpenGLFramebuffer : public Framebuffer {
	public:
		OpenGLFramebuffer(const FBSpecification& spec);
		virtual ~OpenGLFramebuffer();
		void Invalidate();
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual void AddColorAttachment(const FBTextureSpecification& spec) override;
		virtual void AddColorAttachment() override;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index) const override { return m_ColorAttachments[index]; }
		virtual const FBSpecification& GetSpecification() const override { return m_Specification; }
	private:
		uint32_t m_FBO{ 0u };
		uint32_t m_ColorAttachment{ 0u };
		uint32_t m_DepthAttachment{ 0u };
		FBSpecification m_Specification;
		FBTextureSpecification m_DepthAttachmentSpecification = FBTextureFormat::None;
		std::vector<FBTextureSpecification> m_ColorAttachmentSpecifications;
		std::vector<uint32_t> m_ColorAttachments;
	};
}
