#pragma once

#include <vector>
#include <memory>

namespace Aho {
	enum class FBTextureFormat {
		None = 0,
		// Color
		RGBA8,
		RED_INTEGER,
		// Depth/stencil
		DEPTH24STENCIL8,
		// Defaults
		Depth = DEPTH24STENCIL8
	};
	
	enum class FBTexDataFormat {

	};

	enum class FBTextureFilter {

	};

	enum class FBTexPara {
		// Target:
		Texture1D, Texture2D, Texture3D, TextureCubemap,
		// Wrapping
		WrapS, WrapT,
		Clamp, Repeat, MirrorRepeat,
		// Filter
		FilterMin, FilterMag,
		Nearest, Linear, NearestMipmapNearest, LinearMipmapLinear, NearestMipmapLinear, LinearMipmapNearest,
		// Mipmap level
		//LevelBase, LevelMax,
	};

	struct FBTextureSpecification {
		FBTextureSpecification() = default;
		FBTextureSpecification(const FBTextureFormat& format) : TextureFormat(format) {}
		FBTextureFormat TextureFormat = FBTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct FBAttachmentSpecification {
		FBAttachmentSpecification() = default;
		FBAttachmentSpecification(const std::initializer_list<FBTextureSpecification>& attachments)
			: Attachments(attachments) {
		}
		std::vector<FBTextureSpecification> Attachments;
	};

	struct FBSpecification {
		uint32_t Width = 0, Height = 0;
		FBAttachmentSpecification Attachments;
		uint32_t Samples = 1; // what for?
		bool SwapChainTarget = false; // for vulkan
		// Temporary !!!
		//uint32_t rendererID;  // what for?
	};

	class AHO_API Framebuffer {
	public:
		virtual ~Framebuffer() = default;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void AddColorAttachment(const FBTextureSpecification& spec) = 0;
		virtual void AddColorAttachment() = 0;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;
		virtual const FBSpecification& GetSpecification() const = 0;
		static std::shared_ptr<Framebuffer> Create(const FBSpecification& spec);
	};
}

