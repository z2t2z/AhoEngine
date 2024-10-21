#pragma once

#include "Runtime/Function/Renderer/Texture.h"

#include <glad/glad.h>

namespace Aho {
	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D() = default;
		OpenGLTexture2D(const TextureSpecification& specification);
		OpenGLTexture2D(const std::string& path, bool flipOnLoad = false, bool grayScale = true);
		virtual ~OpenGLTexture2D();
		virtual void Invalidate() { AHO_CORE_ASSERT("Should not be called for now!"); };
		virtual TextureSpecification& GetSpecification() override { return m_Specification; }
		virtual void Reload(const std::string& path) {}
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetTextureID() const override { return m_TextureID; }
		virtual const std::string& GetPath() const override { return m_Path; }
		virtual void SetTextureID(uint32_t id) { m_TextureID = id; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(uint32_t slot = 0) const override;
		virtual bool IsLoaded() const override { return m_IsLoaded; }
		virtual bool operator==(const Texture& other) const override {
			return m_TextureID == other.GetTextureID();
		}
	private:
		int m_MipmapLevels{ 0 };
		std::string m_Path;
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_TextureID;	// texture ID
		GLenum m_InternalFormat, m_DataFormat;
	};
}