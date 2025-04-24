#include "Ahopch.h"
#include "OpenGLTexture.h"
#include "Runtime/Core/Core.h"

#include "stb_image.h"
#include <gli/gli.hpp>

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
		glObjectLabel(GL_TEXTURE, m_TextureID, -1, m_Specification.debugName.c_str());

		//m_Specification.height = m_Specification.height;
		//m_Specification.width = m_Specification.width;
		if (m_Specification.target == TexTarget::TextureCubemap) {
			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_Specification.width, m_Specification.width, 0, dataFormat, dataType, nullptr);
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
		auto filename = std::filesystem::path(path).extension().string();
		if (filename.find(".dds") != std::string::npos) {
			LoadGli(path);
			return;
		}

		const std::vector<std::string> HDRextensions = { ".hdr", ".exr" }; // TODO: .exr not supported yet
		bool isHDR = std::ranges::any_of(HDRextensions, [&filename](const std::string& ext) {
			return filename.find(ext) != std::string::npos;
		});

		stbi_set_flip_vertically_on_load(flipOnLoad);
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
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
		//glTexImage2D(GL_TEXTURE_2D, 0, Utils::GetGLParam(spec.internalFormat), spec.width, spec.height, 0, Utils::GetGLParam(spec.dataFormat), Utils::GetGLParam(spec.dataType), data);
		//glGenerateMipmap(GL_TEXTURE_2D);

		glTextureStorage2D(m_TextureID, 1, Utils::GetGLParam(spec.internalFormat), spec.width, spec.height);

		//glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTextureSubImage2D(m_TextureID, 0, 0, 0, spec.width, spec.height, Utils::GetGLParam(spec.dataFormat), Utils::GetGLParam(spec.dataType), data);
		//glGenerateMipmap(GL_TEXTURE_2D); // TODO: buggy

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D() {
		glDeleteTextures(1, &m_TextureID);
	}

	// BIG TODO
	uint32_t OpenGLTexture2D::ReadPixel(float u, float v) const {
		AHO_CORE_ASSERT(false);
		return 0;
	}

	uint64_t OpenGLTexture2D::GetTextureHandle() const {
		if (m_TextureHandle == 0) { 
			m_TextureHandle = glGetTextureHandleARB(m_TextureID);
			glMakeTextureHandleResidentARB(m_TextureHandle);
		}
		//GLint maxResidentHandles;
		//glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &maxResidentHandles);
		//AHO_CORE_ERROR("maxResidentHandles : {}", maxResidentHandles); 192
		return m_TextureHandle;
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size) {
		//uint32_t bpp = m_Specification.internalFormat == TexInterFormat::RGBA8 ? 4 : 3;
		//AHO_CORE_ASSERT(size == m_Specification.width * m_Specification.height * bpp, "Inconsistent data format");
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Specification.width, m_Specification.height, Utils::GetGLParam(m_Specification.dataFormat), GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::ClearTexImage(const void* data) {
		glClearTexImage(m_TextureID, 0, Utils::GetGLParam(m_Specification.dataFormat), Utils::GetGLParam(m_Specification.dataType), data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_TextureID);
	}

	int OpenGLTexture2D::LoadGli(const std::string& Filename) {
		gli::texture Texture = gli::load(Filename);
		if (Texture.empty()) {
			AHO_CORE_ERROR("Load texture failed at `{}`", Filename);
			return 0;
		}

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());

		auto& spec = m_Specification;

		//spec.internalFormat = Format.Internal;
		//spec.dataFormat = Format.External;
		//spec.dataType = Format.Type;

		//GLuint m_TextureID = 0;
		glGenTextures(1, &m_TextureID);
		glBindTexture(Target, m_TextureID);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

		glm::tvec3<GLsizei> const Extent(Texture.extent());
		GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

		switch (Texture.target()) {
			case gli::TARGET_1D:
				glTexStorage1D(
					Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
				break;
			case gli::TARGET_1D_ARRAY:
			case gli::TARGET_2D:
			case gli::TARGET_CUBE:
				glTexStorage2D(
					Target, static_cast<GLint>(Texture.levels()), Format.Internal,
					Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
				break;
			case gli::TARGET_2D_ARRAY:
			case gli::TARGET_3D:
			case gli::TARGET_CUBE_ARRAY:
				glTexStorage3D(
					Target, static_cast<GLint>(Texture.levels()), Format.Internal,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
				break;
			default:
				assert(0);
				break;
		}

		for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
			for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
				for (std::size_t Level = 0; Level < Texture.levels(); ++Level) {
					GLsizei const LayerGL = static_cast<GLsizei>(Layer);
					glm::tvec3<GLsizei> Extent(Texture.extent(Level));
					Target = gli::is_target_cube(Texture.target())
						? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
						: Target;

					switch (Texture.target()) {
						case gli::TARGET_1D:
							if (gli::is_compressed(Texture.format()))
								glCompressedTexSubImage1D(
									Target, static_cast<GLint>(Level), 0, Extent.x,
									Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
									Texture.data(Layer, Face, Level));
							else
								glTexSubImage1D(
									Target, static_cast<GLint>(Level), 0, Extent.x,
									Format.External, Format.Type,
									Texture.data(Layer, Face, Level));
							break;
						case gli::TARGET_1D_ARRAY:
						case gli::TARGET_2D:
						case gli::TARGET_CUBE:
							if (gli::is_compressed(Texture.format()))
								glCompressedTexSubImage2D(
									Target, static_cast<GLint>(Level),
									0, 0,
									Extent.x,
									Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
									Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
									Texture.data(Layer, Face, Level));
							else
								glTexSubImage2D(
									Target, static_cast<GLint>(Level),
									0, 0,
									Extent.x,
									Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
									Format.External, Format.Type,
									Texture.data(Layer, Face, Level));
							break;
						case gli::TARGET_2D_ARRAY:
						case gli::TARGET_3D:
						case gli::TARGET_CUBE_ARRAY:
							if (gli::is_compressed(Texture.format()))
								glCompressedTexSubImage3D(
									Target, static_cast<GLint>(Level),
									0, 0, 0,
									Extent.x, Extent.y,
									Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
									Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
									Texture.data(Layer, Face, Level));
							else
								glTexSubImage3D(
									Target, static_cast<GLint>(Level),
									0, 0, 0,
									Extent.x, Extent.y,
									Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
									Format.External, Format.Type,
									Texture.data(Layer, Face, Level));
							break;
						default: assert(0); break;
					}
				}
		return m_TextureID;
	
	}
}