#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Aho {
	class OpenGLFramebuffer : public Framebuffer {
	public:
		OpenGLFramebuffer(const FBSpec& spec);
		virtual ~OpenGLFramebuffer();
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual bool Resize(uint32_t width, uint32_t height) override;
		virtual void EnableAttachments(uint32_t start, uint32_t cnt) override;
		virtual void BindCubeMap(Texture* tex, int faceIndex, int attachmentID, int mipLevel) override;
		virtual const std::vector<Texture*>& GetTextureAttachments() override;
		virtual Texture* GetDepthTexture() override;
		virtual Texture* GetTextureAttachment(int index) override;
		virtual Texture* GetTexture(TexType type) override;
		virtual uint32_t ReadPixel(TexType type, uint32_t x, uint32_t y, bool shared = false) override;
		virtual uint32_t GetDepthAttachment() override { AHO_CORE_ASSERT(m_DepthTex); return m_DepthTex->GetTextureID(); }
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
		void Invalidate() override;
		void InvalidateColorAttachment();
	private:
		uint32_t m_FBO{ 0u };
		FBSpec m_Specification;
	private:
		std::vector<GLenum> m_Attchments;
		std::vector<Texture*> m_ColorAttachmentTex;
		Texture* m_DepthTex{ nullptr };
	};

}
