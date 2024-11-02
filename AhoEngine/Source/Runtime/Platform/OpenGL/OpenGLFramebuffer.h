#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Aho {
	class OpenGLFramebuffer : public Framebuffer {
	public:
		OpenGLFramebuffer(const FBSpec& spec);
		virtual ~OpenGLFramebuffer();
		void Invalidate() override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void EnableAttachments(uint32_t start, uint32_t cnt) override;
		virtual void BindSharedColorAttachment(Texture* attachment) override;
		virtual void BindSharedDepthAttachment(Texture* attachment) override;
		virtual void BindCubeMap(Texture* tex, int index, int attachmentID) override;
		virtual Texture* GetDepthTexture() override;
		virtual const std::vector<Texture*>& GetTextureAttachments() override;
		virtual Texture* GetTextureAttachment(int index) override;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, bool shared = false) override;
		virtual uint32_t GetDepthAttachment() override { return m_DepthAttachment; }
		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const override { 
			AHO_CORE_ASSERT(index < m_ColorAttachmentTex.size(), "Out of bound while accessing color attachments");
			return m_ColorAttachmentTex[index]->GetTextureID();
		}
		virtual const uint32_t GetLastColorAttachment() const override {
			AHO_CORE_ASSERT(!m_ColorAttachmentTex.empty(), "Colorattachment is empty!");
			return m_ColorAttachmentTex.back()->GetTextureID();
		}
		virtual const FBSpec& GetSpecification() const override { return m_Specification; }
	private:
		void InvalidateColorAttachment();
		void RebindSharedAttachments();
	private:
		uint32_t m_FBO{ 0u };
		uint32_t m_DepthAttachment{ 0u };
	private:
		FBSpec m_Specification;
		TexSpec m_DepthAttachmentSpecification;
	private:
		std::vector<GLenum> m_Attchments;
		std::vector<Texture*> m_ColorAttachmentTex;
		std::vector<Texture*> m_SharedAttachmentTex;
		Texture* m_DepthTex{ nullptr };
		Texture* m_SharedDepthTex{ nullptr };
	};

}
