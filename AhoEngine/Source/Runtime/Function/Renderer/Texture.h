#pragma once

#include "Runtime/Core/Core.h"

#include <string>
#include <memory>


namespace Aho {
	enum class ImageFormat {
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	enum class TextureType {
		None = 0,
		Diffuse,
		Normal,
		Specular,
		Height,
		Roughness,
		Metalic,
		AO,
		Depth
		//Metalness,
		// TODO
	};

	struct TextureSpecification {
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat InternalFormat = ImageFormat::RGBA8;
		ImageFormat Format = ImageFormat::RGBA8;
		int mipmapLevels{ 0 };
	};

	class Texture {
	public:
		Texture() = default;
		Texture(TextureSpecification spec) : m_Specification(spec) {}
		virtual ~Texture() = default;
		virtual void Invalidate() = 0;
		virtual void SetTextureSpecification(TextureSpecification& spec) { m_Specification = spec; }
		virtual TextureSpecification& GetSpecification() = 0;
		virtual void Reload(const std::string& path) = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetTextureID() const = 0;
		virtual TextureType GetTextureType() const { return m_TextureType; }
		virtual void SetTextureType(const TextureType type) { m_TextureType = type; }
		virtual void SetTextureID(uint32_t id) = 0;
		virtual const std::string& GetPath() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool IsLoaded() const = 0;
		virtual bool operator==(const Texture& other) const = 0;
	protected:
		TextureSpecification m_Specification;
		TextureType m_TextureType{ TextureType::None };
	};

	class Texture2D : public Texture {
	public:
		Texture2D(TextureSpecification spec) : Texture(spec) {}
		Texture2D() = default;
		static std::shared_ptr<Texture2D> Create(const TextureSpecification& specification);
		static std::shared_ptr<Texture2D> Create(const std::string& path, bool filpOnLoad = false, bool grayScale = false);
		static std::shared_ptr<Texture2D> Create();
	};

}