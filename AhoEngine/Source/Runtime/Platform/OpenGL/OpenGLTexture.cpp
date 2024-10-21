#include "Ahopch.h"
#include "OpenGLTexture.h"
#include "Runtime/Core/Core.h"
#include "stb_image.h"

namespace Aho {
	namespace Utils {
		static GLenum HazelImageFormatToGLDataFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::RGB8:  return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
				case ImageFormat::RGBA32F: return GL_RGB32F;
			}

			AHO_CORE_ASSERT(false);
			return 0;
		}

		static GLenum HazelImageFormatToGLInternalFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::RGB8:  return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
				case ImageFormat::RGBA32F: return GL_RGB32F;
			}

			AHO_CORE_ASSERT(false);
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: Texture2D(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height) {
		m_InternalFormat = Utils::HazelImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::HazelImageFormatToGLDataFormat(m_Specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
		glTextureStorage2D(m_TextureID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Big TODO
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool flipOnLoad, bool grayScale) : m_Path(path) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(flipOnLoad);
		stbi_uc* data = nullptr;
		data = stbi_load(path.c_str(), &width, &height, &channels, grayScale);
		if (data) {
			m_IsLoaded = true;
			m_Width = width;
			m_Height = height;
			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4) {
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3) {
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}
			else if (channels == 1) {
				internalFormat = GL_R8; 
				dataFormat = GL_RED;    
			}
			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			AHO_CORE_ASSERT(internalFormat != 0 && dataFormat != 0, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
			glTextureStorage2D(m_TextureID, 1, internalFormat, m_Width, m_Height);

			glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		}
		else {
			AHO_CORE_ERROR("Loading texture failed from path: " + path);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D() {
		glDeleteTextures(1, &m_TextureID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size) {
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		AHO_CORE_ASSERT(size == m_Width * m_Height * bpp, "Inconsistent data format");
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_TextureID);
	}
}