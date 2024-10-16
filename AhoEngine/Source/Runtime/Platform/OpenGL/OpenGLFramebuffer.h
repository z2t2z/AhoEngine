#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Aho {
	// Not a good way I guess, for framebuffer use only
	class FBTexture : public Texture {
	public:
		FBTexture() = default;
		FBTexture(uint32_t id) : m_TextureID(id) {}
		~FBTexture() = default;
		void Bind(uint32_t slot) const override {
			glBindTextureUnit(slot, m_TextureID);
		}
		virtual const TextureSpecification& GetSpecification() const override { TextureSpecification v; return v; }
		virtual void Reload(const std::string& path) override {}
		virtual uint32_t GetWidth() const override { return 0u; }
		virtual uint32_t GetHeight() const override { return 0u; }
		virtual uint32_t GetTextureID() const override { return m_TextureID; }
		virtual void SetTextureID(uint32_t id) override { m_TextureID = id; }
		virtual const std::string& GetPath() const override { return "error"; }
		virtual void SetData(void* data, uint32_t size) override {}
		virtual bool IsLoaded() const override { return true; }
		virtual bool operator==(const Texture& other) const override { 
			return m_TextureID == other.GetTextureID(); 
		}
	private:
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
		virtual std::shared_ptr<Texture> GetDepthTexture() override;
		virtual std::vector<std::shared_ptr<Texture>> GetTextureAttachments() override;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y) override;
		virtual uint32_t GetDepthAttachment() override { return m_DepthAttachment; }
		virtual void AddColorAttachment(const FBTextureSpecification& spec) override;
		virtual void AddColorAttachment() override;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;
		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const override { 
			AHO_CORE_ASSERT(index < m_ColorAttachments.size(), "Out of bound while accessing color attachments");
			return m_ColorAttachments[index];
		}
		virtual const uint32_t GetLastColorAttachment() const override {
			AHO_CORE_ASSERT(!m_ColorAttachments.empty(), "Colorattachment is empty!");
			return m_ColorAttachments.back();
		}
		virtual const FBSpecification& GetSpecification() const override { return m_Specification; }
	private:
		uint32_t m_FBO{ 0u };
		uint32_t m_ColorAttachment{ 0u };
		uint32_t m_DepthAttachment{ 0u };
	private:
		FBSpecification m_Specification;
		FBTextureSpecification m_DepthAttachmentSpecification;
		std::vector<FBTextureSpecification> m_ColorAttachmentSpecifications;
		std::vector<uint32_t> m_ColorAttachments;
	private:
		std::vector<std::shared_ptr<Texture>> m_ColorAttachmentTex;
		std::shared_ptr<Texture> m_DepthTex;
	};
}
