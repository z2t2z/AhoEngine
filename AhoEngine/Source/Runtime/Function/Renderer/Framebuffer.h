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
		virtual bool Resize(uint32_t width, uint32_t height) = 0;
		virtual void SetShouldResizeWithViewport(bool state) { m_ResizeWithViewport = state; }
	public:
		static std::shared_ptr<Framebuffer> Create(const FBSpec& spec);
	protected:
		bool m_ResizeWithViewport{ true };
	};
}

