#include "Ahopch.h"
#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Aho {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {
		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static bool IsDepthFormat(FBTextureFormat format) {
			switch (format) {
				case FBTextureFormat::DEPTH24STENCIL8:  return true;
			}
			return false;
		}
		/*
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			target, mipmaplevel, internalFormat, width, height, boarder, dataFormat, dataType, initialData
		*/
		static GLenum FBOTextureFormat(FBTextureFormat format) {
			switch (format) {
				case FBTextureFormat::RGBA8:       return GL_RGBA8;
				case FBTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}
			AHO_CORE_ASSERT(false, "FBOTextureFormat");
			return 0;
		}

		GLuint GetGLParam(FBTarget param) {
			switch (param) {
				case FBTarget::Texture1D:
					return GL_TEXTURE_1D;
				case FBTarget::Texture2D:
					return GL_TEXTURE_2D;
				case FBTarget::Texture3D:
					return GL_TEXTURE_3D;
				case FBTarget::TextureCubemap:
					return GL_TEXTURE_CUBE_MAP;
				default:
					return GL_TEXTURE_2D;
			}
		}

		GLuint GetGLParam(FBWrapMode param) {
			switch (param) {
				case FBWrapMode::Clamp:
					return GL_CLAMP_TO_EDGE;
				case FBWrapMode::Repeat:
					return GL_REPEAT;
				case FBWrapMode::MirrorRepeat:
					return GL_MIRRORED_REPEAT;
				default:
					return GL_NONE;
			}
		}

		GLuint GetGLParam(FBFilterMode param) {
			switch (param) {
				case FBFilterMode::Nearest:
					return GL_NEAREST;
				case FBFilterMode::Linear:
					return GL_LINEAR;
				case FBFilterMode::NearestMipmapNearest:
					return GL_NEAREST_MIPMAP_NEAREST;
				case FBFilterMode::LinearMipmapLinear:
					return GL_LINEAR_MIPMAP_LINEAR;
				case FBFilterMode::NearestMipmapLinear:
					return GL_NEAREST_MIPMAP_LINEAR;
				case FBFilterMode::LinearMipmapNearest:
					return GL_LINEAR_MIPMAP_NEAREST;
				default:
					return GL_NONE;
			}
		}

		GLint GetGLParam(FBInterFormat format) {
			switch (format) {
				case FBInterFormat::RGB8:
					return GL_RGB8;
				case FBInterFormat::RGBA8:
					return GL_RGBA8;
				case FBInterFormat::RGB16F:
					return GL_RGB16F;
				case FBInterFormat::RGBA16F:
					return GL_RGBA16F;
				case FBInterFormat::Depth24:
					return GL_DEPTH_COMPONENT24;
				case FBInterFormat::Depth32F:
					return GL_DEPTH_COMPONENT32F;
				default:
					return GL_RGB8;
			}
		}

		GLenum GetGLParam(FBDataFormat format) {
			switch (format) {
				case FBDataFormat::RGB:
					return GL_RGB;
				case FBDataFormat::RGBA:
					return GL_RGBA;
				case FBDataFormat::DepthComponent:
					return GL_DEPTH_COMPONENT;
				default:
					return GL_RGB;
			}
		}

		GLenum GetGLParam(FBDataType type) {
			switch (type) {
				case FBDataType::UnsignedByte:
					return GL_UNSIGNED_BYTE;
				case FBDataType::Float:
					return GL_FLOAT;
				case FBDataType::UnsignedInt:
					return GL_UNSIGNED_INT;
				default:
					return GL_UNSIGNED_BYTE;
			}
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FBSpecification& spec)
		: m_Specification(spec) {
		for (auto spec : m_Specification.Attachments.Attachments) {
			if (!Utils::IsDepthFormat(spec.TextureFormat)) {
				m_ColorAttachmentSpecifications.push_back(spec);
			}
			else {
				m_DepthAttachmentSpecification = spec;
			}
		}
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
		glDeleteFramebuffers(1, &m_FBO);
	}

	void OpenGLFramebuffer::Invalidate() {
		if (m_FBO) {
			glDeleteFramebuffers(1, &m_FBO);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);
		}
		glCreateFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// TODO: this is the old way
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		m_ColorAttachments.clear();
		auto temp = m_ColorAttachmentSpecifications;
		m_ColorAttachmentSpecifications.clear();
		for (const auto& spec : temp) {
			AddColorAttachment(spec);
		}
		AHO_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
			AHO_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	uint32_t OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) {
		if (attachmentIndex >= m_ColorAttachments.size()) {
			AHO_CORE_ERROR("Attachment index out of bound: {}", attachmentIndex);
			return 0u;
		}
		if (x < 0 || y < 0 || x >= m_Specification.Width || y >= m_Specification.Height) {
			AHO_CORE_ERROR("Attempting to read pixel data from an invalid position: {0}, {1}", x, y);
			return 0u;
		}
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		uint32_t pixelData;
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixelData);
		return pixelData;
	}

	// TODO: Mipmap level, filtering method
	void OpenGLFramebuffer::AddColorAttachment(const FBTextureSpecification& spec) {
		uint32_t _;
		m_ColorAttachments.emplace_back(_);
		m_ColorAttachmentSpecifications.push_back(spec);
		auto target = Utils::GetGLParam(spec.target);
		auto internalFormat = Utils::GetGLParam(spec.internalFormat);
		auto dataFormat = Utils::GetGLParam(spec.dataFormat);
		auto dataType = Utils::GetGLParam(spec.dataType);
		auto wrapModeS = Utils::GetGLParam(spec.wrapModeS);
		auto wrapModeT = Utils::GetGLParam(spec.wrapModeT);
		auto filterModeMin = Utils::GetGLParam(spec.filterModeMin);
		auto filterModeMag = Utils::GetGLParam(spec.filterModeMag);

		glCreateTextures(target, 1, &m_ColorAttachments.back());
		glBindTexture(target, m_ColorAttachments.back());
		glTexImage2D(target, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, nullptr);
		if (wrapModeS) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModeS);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModeT);
		}
		if (filterModeMag) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterModeMin);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterModeMag);
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size() - 1, target, m_ColorAttachments.back(), 0);
		// TODO: why the followings are not working?
		//int mipLevels = 1 + static_cast<int>(std::floor(std::log2(std::max(m_Specification.Width, m_Specification.Height))));
		//glCreateTextures(GL_TEXTURE_2D, mipLevels, &m_ColorAttachments.back());
		//glTextureStorage2D(m_ColorAttachments.back(), 1, GL_RGBA, m_Specification.Width, m_Specification.Height);
		//glTextureParameteri(m_ColorAttachments.back(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTextureParameteri(m_ColorAttachments.back(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size() - 1, GL_TEXTURE_2D, m_ColorAttachments.back(), 0);
	}

	void OpenGLFramebuffer::AddColorAttachment() {
		AHO_CORE_ASSERT(!m_ColorAttachments.empty(), "Attempting to add a color attachment to an incomplete frame buffer");
		const auto spec = m_ColorAttachmentSpecifications[0];
		AddColorAttachment(spec);
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value) {
		AHO_CORE_ERROR("Not implemented yet");
		return;
		AHO_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index is out of bound!");
		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0,
			Utils::FBOTextureFormat(spec.TextureFormat), GL_INT, &value);
	}

}