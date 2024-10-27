#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Aho {
	namespace Utils {
		Texture* CreateNoiseTexture(int siz);
	}
	// Not a good way I guess, for framebuffer use only
	class FBTexture : public Texture {
	public:
		FBTexture() = default;
		FBTexture(uint32_t id) : m_TextureID(id) {}
		~FBTexture() { Invalidate(); }
		void Invalidate() override { glDeleteTextures(1, &m_TextureID); }
		void Bind(uint32_t slot) const override { glBindTextureUnit(slot, m_TextureID); }
		virtual TextureSpecification& GetSpecification() override { return m_Specification; }
		virtual void Reload(const std::string& path) override {}
		virtual uint32_t GetWidth() const override { return 0u; }
		virtual uint32_t GetHeight() const override { return 0u; }
		virtual uint32_t GetTextureID() const override { return m_TextureID; }
		virtual void SetTextureID(uint32_t id) override { m_TextureID = id; }
		virtual const std::string& GetPath() const override { return "error at FBTexture"; }
		virtual void SetData(void* data, uint32_t size) override {}
		virtual bool IsLoaded() const override { return true; }
		virtual bool operator==(const Texture& other) const override { return m_TextureID == other.GetTextureID(); }
	private:
		int m_MipmapLevels{ 0 };
		uint32_t m_TextureID{ 0u };
	};
	

	class OpenGLFramebuffer : public Framebuffer {
	public:
		OpenGLFramebuffer(const FBSpecification& spec);
		virtual ~OpenGLFramebuffer();
		void Invalidate() override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual Texture* GetDepthTexture() override;
		virtual const std::vector<Texture*>& GetTextureAttachments() override;
		virtual Texture* GetTextureAttachment(int index) override;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y) override;
		virtual uint32_t GetDepthAttachment() override { return m_DepthAttachment; }
		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const override { 
			AHO_CORE_ASSERT(index < m_ColorAttachmentTex.size(), "Out of bound while accessing color attachments");
			return m_ColorAttachmentTex[index]->GetTextureID();
		}
		virtual const uint32_t GetLastColorAttachment() const override {
			AHO_CORE_ASSERT(!m_ColorAttachmentTex.empty(), "Colorattachment is empty!");
			return m_ColorAttachmentTex.back()->GetTextureID();
		}
		virtual const FBSpecification& GetSpecification() const override { return m_Specification; }
	private:
		virtual void InvalidateColorAttachment();
	private:
		uint32_t m_FBO{ 0u };
		uint32_t m_ColorAttachment{ 0u };
		uint32_t m_DepthAttachment{ 0u };
	private:
		FBSpecification m_Specification;
		FBTextureSpecification m_DepthAttachmentSpecification;
		std::vector<FBTextureSpecification> m_ColorAttachmentSpecifications;
	private:
		std::vector<GLenum> m_Attchments;
		std::vector<Texture*> m_ColorAttachmentTex;
		Texture* m_DepthTex{ nullptr };
	};
}
