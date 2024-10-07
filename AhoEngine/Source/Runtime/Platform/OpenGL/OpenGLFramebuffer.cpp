#include "Ahopch.h"
#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Aho {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {
		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height) {
			bool multisampled = samples > 1;
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FBTextureFormat format) {
			switch (format) {
				case FBTextureFormat::DEPTH24STENCIL8:  return true;
			}
			return false;
		}

		static GLenum FBOTextureFormatToGL(FBTextureFormat format) {
			switch (format) {
				case FBTextureFormat::RGBA8:       return GL_RGBA8;
				case FBTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}
			AHO_CORE_ASSERT(false, "FBOTextureFormatToGL");
			return 0;
		}

		static GLenum FBTextureTargetNameToGL(FBTexPara para) {
			switch (para) {
				case FBTexPara::Texture1D:				return GL_TEXTURE_1D;
				case FBTexPara::Texture2D:				return GL_TEXTURE_2D;
				case FBTexPara::Texture3D:				return GL_TEXTURE_3D;
				case FBTexPara::TextureCubemap:			return GL_TEXTURE_CUBE_MAP;
			
				case FBTexPara::WrapS:					return GL_TEXTURE_WRAP_S;
				case FBTexPara::WrapT:					return GL_TEXTURE_WRAP_T;

				case FBTexPara::Clamp:					return GL_CLAMP_TO_EDGE;
				case FBTexPara::Repeat:					return GL_REPEAT;
				case FBTexPara::MirrorRepeat:			return GL_MIRRORED_REPEAT;

				case FBTexPara::FilterMin:				return GL_TEXTURE_MIN_FILTER;
				case FBTexPara::FilterMag:				return GL_TEXTURE_MAG_FILTER;

				case FBTexPara::Nearest:				return GL_NEAREST;
				case FBTexPara::Linear:					return GL_LINEAR;
				case FBTexPara::NearestMipmapNearest:	return GL_NEAREST_MIPMAP_NEAREST;
				case FBTexPara::LinearMipmapLinear:		return GL_LINEAR_MIPMAP_LINEAR;
				case FBTexPara::NearestMipmapLinear:	return GL_NEAREST_MIPMAP_LINEAR;
				case FBTexPara::LinearMipmapNearest:	return GL_LINEAR_MIPMAP_NEAREST;
				case FBTexPara::LevelBase:				return GL_TEXTURE_BASE_LEVEL;
				case FBTexPara::LevelMax:				return GL_TEXTURE_MAX_LEVEL;
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
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachments.back());
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachments.back());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size() - 1, GL_TEXTURE_2D, m_ColorAttachments.back(), 0);
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
		AHO_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index is out of bound!");
		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0,
			Utils::FBOTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
	}

}