#pragma once

#include "Runtime/Core/Math/Math.h"
#include "Runtime/Function/Renderer/Texture.h"
#include "Runtime/Core/Log/Log.h"
#include <glad/glad.h>


namespace Aho {
	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D() = default;
		OpenGLTexture2D(const TexSpec& specification);
		OpenGLTexture2D(const std::string& path, bool flipOnLoad = false, bool grayScale = true);
		virtual ~OpenGLTexture2D();
		virtual void Reload(const std::string& path) {}
		virtual void Invalidate() override;
		virtual TexSpec& GetSpecification() override { return m_Specification; }
		virtual uint32_t GetWidth() const override { return m_Specification.width; }
		virtual uint32_t GetHeight() const override { return m_Specification.height; }
		virtual uint32_t GetTextureID() const override { return m_TextureID; }
		virtual uint32_t ReadPixel(float u, float v) const override;
		virtual uint64_t GetTextureHandle() const override;
		virtual const std::string& GetPath() const override { return m_Path; }
		virtual void SetTextureID(uint32_t id) { m_TextureID = id; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void ClearTexImage(const void* data) override;
		virtual void Bind(uint32_t slot = 0) const override;
		virtual bool operator==(const Texture& other) const override {
			return m_TextureID == other.GetTextureID();
		}
	private:
		int LoadGli(const std::string& path);
		std::string m_Path;
		uint32_t m_TextureID{ 0 };	// texture ID
		mutable GLuint64 m_TextureHandle{ 0 };
	};

	namespace Utils {
		static int CalculateMaximumMipmapLevels(int siz) {
			int mipLevels = (int)floor(log2(siz)) + 1;
			return mipLevels;
		}
		/*
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			target, mipmaplevel, internalFormat, width, height, boarder, dataFormat, dataType, initialData
		*/
		// TODO: Just used the same enum

		static GLuint GetGLParam(TexTarget param) {
			switch (param) {
			case TexTarget::Texture1D:
				return GL_TEXTURE_1D;
			case TexTarget::Texture2D:
				return GL_TEXTURE_2D;
			case TexTarget::Texture3D:
				return GL_TEXTURE_3D;
			case TexTarget::TextureCubemap:
				return GL_TEXTURE_CUBE_MAP;
			default:
				return GL_TEXTURE_2D;
			}
		}

		static GLuint GetGLParam(TexWrapMode param) {
			switch (param) {
			case TexWrapMode::Clamp:
				return GL_CLAMP_TO_EDGE;
			case TexWrapMode::Repeat:
				return GL_REPEAT;
			case TexWrapMode::MirrorRepeat:
				return GL_MIRRORED_REPEAT;
			default:
				return GL_NONE;
			}
		}

		static GLuint GetGLParam(TexFilterMode param) {
			switch (param) {
			case TexFilterMode::Nearest:
				return GL_NEAREST;
			case TexFilterMode::Linear:
				return GL_LINEAR;
			case TexFilterMode::NearestMipmapNearest:
				return GL_NEAREST_MIPMAP_NEAREST;
			case TexFilterMode::LinearMipmapLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			case TexFilterMode::NearestMipmapLinear:
				return GL_NEAREST_MIPMAP_LINEAR;
			case TexFilterMode::LinearMipmapNearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			default:
				return GL_NONE;
			}
		}

		static GLint GetGLParam(TexInterFormat format) {
			switch (format) {
			case TexInterFormat::RED:
				return GL_RED;
			case TexInterFormat::UINT:
				return GL_R32UI;
			case TexInterFormat::RG8:
				return GL_RG8;
			case TexInterFormat::RG16F:
				return GL_RG16F;
			case TexInterFormat::RED32F:
				return GL_R32F;
			case TexInterFormat::RGB8:
				return GL_RGB8;
			case TexInterFormat::RGBA8:
				return GL_RGBA8;
			case TexInterFormat::RGB16F:
				return GL_RGB16F;
			case TexInterFormat::RGBA16F:
				return GL_RGBA16F;
			case TexInterFormat::RGBA32F:
				return GL_RGBA32F;
			case TexInterFormat::Depth24:
				return GL_DEPTH_COMPONENT24;
			case TexInterFormat::Depth32:
				return GL_DEPTH_COMPONENT32F;
			case TexInterFormat::Depth24Stencil8:
				return GL_DEPTH24_STENCIL8;
			default:
				return GL_RGB8;
			}
		}

		static GLenum GetGLParam(TexDataFormat format) {
			switch (format) {
			case TexDataFormat::RED:
				return GL_RED;
			case TexDataFormat::RG:
				return GL_RG;
			case TexDataFormat::UINT:
				return GL_RED_INTEGER;
			case TexDataFormat::RGB:
				return GL_RGB;
			case TexDataFormat::RGBA:
				return GL_RGBA;
			case TexDataFormat::DepthComponent:
				return GL_DEPTH_COMPONENT;
			case TexDataFormat::DepthStencil:
				return GL_DEPTH_STENCIL;
			default:
				return GL_RGB;
			}
		}

		static GLenum GetGLParam(TexDataType type) {
			switch (type) {
			case TexDataType::UnsignedByte:
				return GL_UNSIGNED_BYTE;
			case TexDataType::Float:
				return GL_FLOAT;
			case TexDataType::UnsignedInt:
				return GL_UNSIGNED_INT;
			case TexDataType::UnsignedInt248:
				return GL_UNSIGNED_INT_24_8;
			default:
				return GL_UNSIGNED_BYTE;
			}
		}

		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static Texture* CreateNoiseTexture(int siz) {
			std::vector<glm::vec3> ssaoNoise;
			for (int i = 0; i < siz; i++) {
				glm::vec3 noise = GenerateRandomVec3();
				noise.z = 0.0f;
				ssaoNoise.push_back(noise);
			}
			uint32_t id;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			Texture* noiseTex = new OpenGLTexture2D(TexSpec());
			noiseTex->SetTextureID(id);
			return noiseTex;
		}

		static bool IsDepthFormat(TexDataFormat format) {
			return format == TexDataFormat::DepthComponent || format == TexDataFormat::DepthStencil;
		}
	} // namespace Utils
} // namespace Aho