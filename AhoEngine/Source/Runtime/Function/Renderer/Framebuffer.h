#pragma once
#include "Texture.h"
#include <vector>
#include <memory>

namespace Aho {
	// Framebuffer specification
	struct FBSpec {
		FBSpec() = default;
		FBSpec(uint32_t width, uint32_t height, const std::initializer_list<TexSpec>& attachments)
			: Width(width), Height(height), Attachments(attachments) {
		}
		uint32_t Width = 0, Height = 0;
		std::vector<TexSpec> Attachments;
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Texture* GetDepthTexture() = 0;
		virtual void EnableAttachments(uint32_t start, uint32_t cnt = 0) = 0;
		virtual void BindSharedColorAttachment(Texture* attachment) = 0;
		virtual void BindSharedDepthAttachment(Texture* attachment) = 0;
		virtual const std::vector<Texture*>& GetTextureAttachments() = 0;
		virtual Texture* GetTextureAttachment(int index) = 0;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, bool shared = false) = 0;
		virtual uint32_t GetDepthAttachment() = 0;
		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;
		virtual const uint32_t GetLastColorAttachment() const = 0;
		virtual const FBSpec& GetSpecification() const = 0;
		static std::shared_ptr<Framebuffer> Create(const FBSpec& spec);
	};
}

