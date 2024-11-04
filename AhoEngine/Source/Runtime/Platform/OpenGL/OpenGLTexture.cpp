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
		if (m_Specification.target == TexTarget::TextureCubemap) {
			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_Specification.width, m_Specification.height, 0, dataFormat, dataType, nullptr);
			}
		}
		else {
			glTexImage2D(target, 0, internalFormat, m_Specification.width, m_Specification.height, 0, dataFormat, dataType, nullptr);
		}

		if (wrapModeS) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModeS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModeT);
			if (m_Specification.target == TexTarget::TextureCubemap) {
				glTexParameteri(target, GL_TEXTURE_WRAP_R, Utils::GetGLParam(m_Specification.wrapModeR));
			}
		}
		if (filterModeMag) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterModeMin);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterModeMag);
		}
		if (mipLevels) {
			if (mipLevels == 1) {
				m_Specification.mipLevels = Utils::CalculateMaximumMipmapLevels(std::max(m_Specification.height, m_Specification.width));
			}
			glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, m_Specification.mipLevels);
			glGenerateMipmap(target);
		}
	
	}

	// Big TODO
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool flipOnLoad, bool grayScale) : m_Path(path) {
		stbi_set_flip_vertically_on_load(flipOnLoad);
		auto filename = std::filesystem::path(path).extension().string();		

		const std::vector<std::string> HDRextensions = { ".hdr", ".exr" }; // TODO: .exr not supported yet
		bool isHDR = std::ranges::any_of(HDRextensions, [&filename](const std::string& ext) {
			return filename.find(ext) != std::string::npos;
		});

		void* data = nullptr;
		data = isHDR ? (void*)stbi_loadf(path.c_str(), &m_Specification.width, &m_Specification.height, &m_Specification.channels, 0)	
			: (void*)stbi_load(path.c_str(), &m_Specification.width, &m_Specification.height, &m_Specification.channels, grayScale);

		if (!data) {
			AHO_CORE_ERROR("Loading texture failed from path: {}", path);
			return;
		}

		auto& spec = m_Specification;

		if (isHDR) {
			spec.internalFormat = TexInterFormat::RGB16F;
			spec.dataFormat = TexDataFormat::RGB;
			spec.dataType = TexDataType::Float;
		}
		else {
			spec.dataType = TexDataType::UnsignedByte;
			if (spec.channels == 4) {
				spec.internalFormat = TexInterFormat::RGBA8;
				spec.dataFormat = TexDataFormat::RGBA;
			}
			else if (spec.channels == 3) {
				spec.internalFormat = TexInterFormat::RGB8;
				spec.dataFormat = TexDataFormat::RGB;
			}
			else if (spec.channels == 2) {
				spec.internalFormat = TexInterFormat::RG8;
				spec.dataFormat = TexDataFormat::RG;
			}
			else if (spec.channels == 1) {
				spec.internalFormat = TexInterFormat::RED;
				spec.dataFormat = TexDataFormat::RED;
			}
			else {
				AHO_CORE_ERROR("Texture format not supported from path: {}", path);
				return;
			}
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
		//glBindTexture(GL_TEXTURE_2D, m_TextureID);
		//glTexImage2D(GL_TEXTURE_2D, 0, Utils::GetGLParam(spec.internalFormat), spec.width, spec.height, 0, Utils::GetGLParam(spec.dataFormat), Utils::GetGLParam(spec.dataType), data);
		//glGenerateMipmap(GL_TEXTURE_2D);

		glTextureStorage2D(m_TextureID, 1, Utils::GetGLParam(spec.internalFormat), spec.width, spec.height);

		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_CLAMP_TO_EDGE);

		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_LINEAR);
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_LINEAR);

		glTextureSubImage2D(m_TextureID, 0, 0, 0, spec.width, spec.height, Utils::GetGLParam(spec.dataFormat), Utils::GetGLParam(spec.dataType), data);
		//glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
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