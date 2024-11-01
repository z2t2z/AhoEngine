#include "Ahopch.h"
#include "OpenGLTexture.h"
#include "Runtime/Core/Core.h"
#include "stb_image.h"

namespace Aho {
	OpenGLTexture2D::OpenGLTexture2D(const TexSpec& specification)
		: Texture2D(specification) {
		Invalidate();
	}

	void OpenGLTexture2D::Invalidate() {
		auto target = Utils::GetGLParam(m_Specification.target);
		auto internalFormat = Utils::GetGLParam(m_Specification.internalFormat);
		auto dataFormat = Utils::GetGLParam(m_Specification.dataFormat);
		auto dataType = Utils::GetGLParam(m_Specification.dataType);
		auto wrapModeS = Utils::GetGLParam(m_Specification.wrapModeS);
		auto wrapModeT = Utils::GetGLParam(m_Specification.wrapModeT);
		auto filterModeMin = Utils::GetGLParam(m_Specification.filterModeMin);
		auto filterModeMag = Utils::GetGLParam(m_Specification.filterModeMag);
		auto& mipLevels = m_Specification.mipLevels;

		if (m_TextureID) {
			glDeleteTextures(1, &m_TextureID);
		}

		glCreateTextures(target, 1, &m_TextureID);
		glBindTexture(target, m_TextureID);

		m_Specification.height = m_Specification.height;
		m_Specification.width = m_Specification.width;
		glTexImage2D(target, 0, internalFormat, m_Specification.width, m_Specification.height, 0, dataFormat, dataType, nullptr);
		if (wrapModeS) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModeS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModeT);
		}
		if (filterModeMag) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterModeMin);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterModeMag);
		}
		if (mipLevels) {
			m_Specification.mipLevels = Utils::CalculateMaximumMipmapLevels(std::max(m_Specification.height, m_Specification.width));
			glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, m_Specification.mipLevels);
			glGenerateMipmap(target);
		}
	
	}

	// Big TODO
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool flipOnLoad, bool grayScale) : m_Path(path) {
		stbi_set_flip_vertically_on_load(flipOnLoad);
		stbi_uc* data = nullptr;
		data = stbi_load(path.c_str(), &m_Specification.width, &m_Specification.height, &m_Specification.channels, grayScale);
		if (data) {
			GLenum internalFormat = 0, dataFormat = 0;
			if (m_Specification.channels == 4) {
				m_Specification.internalFormat = TexInterFormat::RGBA8;
				m_Specification.dataFormat = TexDataFormat::RGBA;
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (m_Specification.channels == 3) {
				m_Specification.internalFormat = TexInterFormat::RGB8;
				m_Specification.dataFormat = TexDataFormat::RGB;
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}
			else if (m_Specification.channels == 2) {
				//m_Specification.internalFormat = TexInterFormat::RG8;
				//m_Specification.dataFormat = TexDataFormat::RG;
				internalFormat = GL_RG8;
				dataFormat = GL_RG;
			}
			else if (m_Specification.channels == 1) {
				m_Specification.internalFormat = TexInterFormat::RED;
				m_Specification.dataFormat = TexDataFormat::RED;
				internalFormat = GL_R8; 
				dataFormat = GL_RED;    
			}

			AHO_CORE_ASSERT(internalFormat != 0 && dataFormat != 0, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
			glTextureStorage2D(m_TextureID, 1, internalFormat, m_Specification.width, m_Specification.height);

			glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Specification.width, m_Specification.height, dataFormat, GL_UNSIGNED_BYTE, data);
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
		uint32_t bpp = m_Specification.internalFormat == TexInterFormat::RGB8 ? 4 : 3;
		AHO_CORE_ASSERT(size == m_Specification.width * m_Specification.height * bpp, "Inconsistent data format");
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Specification.width, m_Specification.height, Utils::GetGLParam(m_Specification.dataFormat), GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_TextureID);
	}
}