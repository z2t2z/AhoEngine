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

	enum class FBInterFormat {
		None, RGB8, RGBA8, RGB16F, RGBA16F, Depth24, Depth32F,
	};

	enum class FBDataFormat {
		None, RGB, RGBA, DepthComponent,
	};

	enum class FBDataType {
		None, UnsignedByte, Float, UnsignedInt
	};

	enum class FBTarget {
		None, Texture1D, Texture2D, Texture3D, TextureCubemap
	};
	enum class FBMipmapLevel {
		None, MipMapLevelBase, MipMapLevelMax
	};
	enum class FBWrapType {
		None, WrapS, WrapT
	};
	enum class FBWrapMode {
		None, Clamp, Repeat, MirrorRepeat
	};
	enum class FBFilterType {
		None, FilterMin, FilterMag
	};
	enum class FBFilterMode {
		None, Nearest, Linear, NearestMipmapNearest, LinearMipmapLinear, NearestMipmapLinear, LinearMipmapNearest
	};
	

	struct FBTextureSpecification {
		FBTextureSpecification() = default;
		FBTextureSpecification(const FBTextureFormat& format) : TextureFormat(format) {}
		FBTextureFormat TextureFormat = FBTextureFormat::None;
		FBInterFormat internalFormat{ FBInterFormat::None };
		FBTarget target{ FBTarget::None };
		FBWrapMode wrapModeS{ FBWrapMode::None };
		FBWrapMode wrapModeT{ FBWrapMode::None };
		FBFilterMode filterModeMin{ FBFilterMode::None };
		FBFilterMode filterModeMag{ FBFilterMode::None };
		FBDataType dataType{ FBDataType::None };
		FBDataFormat dataFormat{ FBDataFormat::None };
	};

	struct FBAttachmentSpecification {
		FBAttachmentSpecification() = default;
		FBAttachmentSpecification(const std::initializer_list<FBTextureSpecification>& attachments)
			: Attachments(attachments) {}
		std::vector<FBTextureSpecification> Attachments;
	};

	struct FBSpecification {
		uint32_t Width = 0, Height = 0;
		FBAttachmentSpecification Attachments;
		uint32_t Samples = 1; // what for?
		bool SwapChainTarget = false; // for vulkan
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual uint32_t ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void AddColorAttachment(const FBTextureSpecification& spec) = 0;
		virtual void AddColorAttachment() = 0;
		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;
		virtual const uint32_t GetLastColorAttachment() const = 0;
		virtual const FBSpecification& GetSpecification() const = 0;
		static std::shared_ptr<Framebuffer> Create(const FBSpecification& spec);
	};
}

