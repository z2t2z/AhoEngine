#include "Ahopch.h"
#include "_Texture.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/AssetManager.h"

#include "stb_image.h"
#include <gli/gli.hpp>
#include <glad/glad.h>

namespace Aho {
	_Texture::_Texture(const TextureConfig& cfg) {
		m_Usage		  = cfg.Usage;
		m_Label		  = cfg.Label;
		m_Dim		  = cfg.Dim;
		m_DataFmt	  = cfg.DataFmt;
		m_DataType	  = cfg.DataType;
		m_InternalFmt = cfg.InternalFmt;
		m_MipLevels	  = cfg.GenMips ? 1 : -1;

		Resize(cfg.Width, cfg.Height);
	}

	_Texture::_Texture(const std::shared_ptr<TextureAsset>& texAsset) {
		auto filename = std::filesystem::path(texAsset->GetPath()).extension().string();
		if (filename.find(".dds") != std::string::npos) {
			GliLoader(texAsset);
		}
		else {
			STBImageLoader(texAsset);
		}
		if (!m_TextureID) {
			AHO_CORE_ERROR("Error occured loading texture resource");
		}
	}

	_Texture::~_Texture() {
		if (m_Handle) {
			glMakeTextureHandleNonResidentARB(m_Handle);
		}
		glDeleteTextures(1, &m_TextureID);
	}

	void _Texture::BindTextureImage(uint32_t pos) const {
		glBindImageTexture(pos, m_TextureID, 0, (m_Dim == TextureDim::Texture2D ? GL_FALSE : GL_TRUE), 0, GL_WRITE_ONLY, m_InternalFmt);
	}

	// TODO: imutable storage
	bool _Texture::Resize(uint32_t width, uint32_t height) {
		if (m_Width == width && m_Height == height) {
			return false;
		}
		m_Width = width; m_Height = height;

		if (m_TextureID) {
			glDeleteTextures(1, &m_TextureID);
		}

		glCreateTextures(m_Dim, 1, &m_TextureID);
		glBindTexture(m_Dim, m_TextureID);
		if (!m_Label.empty()) {
			glObjectLabel(GL_TEXTURE, m_TextureID, -1, m_Label.c_str());
		}

		if (m_Dim == GL_TEXTURE_CUBE_MAP) {
			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_InternalFmt, m_Width, m_Height, 0, m_DataFmt, m_DataType, nullptr);
			}
		}
		else {
			glTexImage2D(m_Dim, 0, m_InternalFmt, m_Width, m_Height, 0, m_DataFmt, m_DataType, nullptr);
		}

		if (m_MipLevels != -1) {
			int maxDim = std::max(m_Width, m_Height);
			m_MipLevels = 1 + static_cast<int>(std::floor(std::log2(maxDim)));
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glTexParameteri(m_Dim, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_Dim, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (m_Dim == GL_TEXTURE_CUBE_MAP) {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		glTexParameteri(m_Dim, GL_TEXTURE_MIN_FILTER, m_MipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(m_Dim, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		return true;
	}

	void _Texture::GliLoader(const std::shared_ptr<TextureAsset>& texAsset) {
		gli::texture Texture = gli::load(texAsset->GetPath());
		if (Texture.empty()) {
			AHO_CORE_ERROR("Failed to load texture from `{}`", texAsset->GetPath());
			return;
		}

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(Target, textureID);
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

		m_TextureID = textureID;
		// TODO, fill other properties
	}

	void _Texture::STBImageLoader(const std::shared_ptr<TextureAsset>& texAsset) {
		static const std::vector<std::string> HDRextensions = { ".hdr", ".exr" }; // TODO: .exr not supported yet
		auto path = texAsset->GetPath();
		bool isHDR = std::ranges::any_of(HDRextensions, 
			[&path](const std::string& ext) {
				return path.find(ext) != std::string::npos;
			});

		stbi_set_flip_vertically_on_load(texAsset->FilpUV());
		void* data = nullptr;
		int width, height, nrChannels;
		data = isHDR ? (void*)stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0)
			         : (void*)stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (!data) {
			AHO_CORE_ERROR("Failed to load texture from `{}`", texAsset->GetPath());
			AHO_CORE_ASSERT(false);
			return;
		}

		GLenum internalFormat;
		GLenum dataFormat;
		if (isHDR) {
			if (nrChannels == 3) {
				internalFormat = GL_RGB16F;
				dataFormat = GL_RGB;
			}
			else if (nrChannels == 4) {
				internalFormat = GL_RGBA16F;
				dataFormat = GL_RGBA;
			}
			else {
				AHO_CORE_ERROR("Unsupported HDR components {} ", nrChannels);
				stbi_image_free(data);
				return;
			}
		}
		else {
			bool gammaCorrection = false; // This is explicitly done in shader
			if (nrChannels == 1) {
				internalFormat = dataFormat = GL_RED;
			}
			else if (nrChannels == 3) {
				internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
				dataFormat = GL_RGB;
			}
			else if (nrChannels == 4) {
				internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
				dataFormat = GL_RGBA;
			}
			else {
				AHO_CORE_ERROR("Unsupported components {} ", nrChannels);
				stbi_image_free(data);
				return;
			}
		}

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
		
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		m_TextureID		= textureID;
		m_Height		= height;
		m_Width			= width;
		m_DataFmt		= (DataFormat)dataFormat;
		m_InternalFmt	= (InternalFormat)internalFormat;
		m_DataType		= isHDR ? DataType::Float : DataType::UByte;
	}

}