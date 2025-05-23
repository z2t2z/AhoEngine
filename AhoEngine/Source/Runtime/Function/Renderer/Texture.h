#pragma once

#include "Runtime/Core/Core.h"

#include <string>
#include <memory>

namespace Aho {
	// Texture parameters
	enum class TexInterFormat {
		None, RED, UINT, RED32F, RG8, RG16F, RGB8, RGBA8, RGB16F, RGBA16F, RGBA32F, Depth24, Depth24Stencil8, Depth32,
	};
	enum class TexDataFormat {
		None, RED, UINT, RG, RGB, RGBA, DepthComponent, DepthStencil
	};
	enum class TexDataType {
		None, UnsignedByte, Float, UnsignedInt, UnsignedInt248
	};
	enum class TexTarget {
		None, Texture1D, Texture2D, Texture3D, TextureCubemap
	};
	enum class TexWrapMode {
		None, Clamp, Repeat, MirrorRepeat
	};
	enum class TexFilterMode {
		None, Nearest, Linear, NearestMipmapNearest, LinearMipmapLinear, NearestMipmapLinear, LinearMipmapNearest
	};

	enum class TexType {
		None = 0,
		Normal,
		Height,
		AO,
		
		// Disney principled
		Albedo,
		Emissive,
		EmissiveScale,

		Subsurface,
		Metallic,
		Specular,
		SpecTint,
		Roughness,
		Anisotropic,
		Sheen,
		SheenTint,
		Clearcoat,
		ClearcoatGloss,
		SpecTrans,
		ior,

		// Framebuffer texture type
		Noise,
		HDR,
		CubeMap,
		CubeMapIrradiance,
		CubeMapSkybox,
		Position,
		Depth,
		LightDepth,
		Entity,
		PBR,	// RGB: Metalness, Roughness, AO
		Irradiance,
		Prefiltering,
		BRDFLUT,
		TransmittanceLUT,
		MultiScattLUT,
		SkyViewLUT,
		AreialPerspectiveLUT,
		PathTracingAccumulate,
		Result	// The final result texture of a render pass
	};


	// Texture specification
	struct TexSpec {
		TexSpec() = default;
		TexType type{ TexType::None };
		TexTarget target{ TexTarget::Texture2D };
		TexInterFormat internalFormat{ TexInterFormat::RGBA8 };
		TexDataFormat dataFormat{ TexDataFormat::RGBA };
		TexDataType dataType{ TexDataType::UnsignedByte };
		TexWrapMode wrapModeS{ TexWrapMode::Clamp };
		TexWrapMode wrapModeT{ TexWrapMode::Clamp };
		TexWrapMode wrapModeR{ TexWrapMode::Clamp };
		TexFilterMode filterModeMin{ TexFilterMode::Linear };
		TexFilterMode filterModeMag{ TexFilterMode::Linear };
		int mipLevels{ 0 };  // 0: No mipmaps; 1: auto calculate: max(logWidth, logHeight); 
		int width{ 1280 }, height{ 720 }, channels{ 4 };
		std::string debugName{ "unnamed" };
	};
 
	class Texture {
	public:
		Texture() = default;
		Texture(TexSpec spec) : m_Specification(spec) {}
		virtual ~Texture() = default;
		virtual void Invalidate() = 0;
		virtual const TexSpec GetTextureSpec() { return m_Specification; }
		virtual void SetTextureSpecification(TexSpec& spec) { m_Specification = spec; }
		virtual TexSpec& GetSpecification() = 0;
		virtual void Reload(const std::string& path) = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetTextureID() const = 0;
		virtual uint32_t ReadPixel(float u, float v) const = 0;
		virtual uint64_t GetTextureHandle() const = 0;
		virtual TexType GetTexType() const { return m_Specification.type; }
		virtual void SetTexType(const TexType type) { m_Specification.type = type; }
		virtual void SetTextureID(uint32_t id) = 0;
		virtual const std::string& GetPath() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void ClearTexImage(const void* data) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool operator==(const Texture& other) const = 0;
		virtual std::string GetDebugName() { return m_Specification.debugName; }
	protected:
		TexSpec m_Specification;
	};

	class Texture2D : public Texture {
	public:
		Texture2D(TexSpec spec) : Texture(spec) {}
		Texture2D() = default;
		static std::shared_ptr<Texture2D> Create(const TexSpec& specification);
		static std::shared_ptr<Texture2D> Create(const std::string& path, bool filpOnLoad = false, bool grayScale = false);
		static std::shared_ptr<Texture2D> Create();
	};

}