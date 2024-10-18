#include "Ahopch.h"
#include "OpenGLFrameBuffer.h"
#include "Runtime/Core/Core.h"

namespace Aho {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {
		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static bool IsDepthFormat(FBDataFormat format) {
			switch (format) {
				case FBDataFormat::DepthComponent:  return true;
			}
			return false;
		}
		/*
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			target, mipmaplevel, internalFormat, width, height, boarder, dataFormat, dataType, initialData
		*/

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
				case FBInterFormat::RED:
					return GL_RED;
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
				case FBDataFormat::RED:
					return GL_RED;
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
		for (const auto& spec : m_Specification.Attachments) {
			if (!Utils::IsDepthFormat(spec.dataFormat)) {
				m_ColorAttachmentSpecifications.push_back(spec);
				m_Attchments.push_back(GL_COLOR_ATTACHMENT0 + m_Attchments.size());
			}
			else {
				m_DepthTex = new FBTexture();
				m_DepthAttachmentSpecification = spec;
			}
		}
		m_ColorAttachmentTex.resize(m_Attchments.size());
		std::fill(m_ColorAttachmentTex.begin(), m_ColorAttachmentTex.end(), new FBTexture());
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {
		if (m_DepthTex) {
			delete m_DepthTex;
		}
		for (auto& tex : m_ColorAttachmentTex) {
			delete tex;
		}
	}

	// TODO: Make these more clear
	void OpenGLFramebuffer::Invalidate() {
		if (m_FBO) {
			glDeleteFramebuffers(1, &m_FBO);
			if (m_DepthTex) {
				m_DepthTex->Invalidate();
			}
			for (auto& tex : m_ColorAttachmentTex) {
				tex->Invalidate();
			}
		}
		// if has depth attachment, invalidate it
		if (m_DepthAttachmentSpecification.dataFormat == FBDataFormat::DepthComponent) {
			glCreateFramebuffers(1, &m_FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
			m_DepthTex->SetTextureID(m_DepthAttachment);
			glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
		}

		// if has color attachments, invalidate them
		if (!m_ColorAttachmentSpecifications.empty()) {
			InvalidateColorAttachment();
			glDrawBuffers(static_cast<GLsizei>(m_Attchments.size()), m_Attchments.data());
		}
		else {
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
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

	Texture* OpenGLFramebuffer::GetDepthTexture() {
		return m_DepthTex;
	}

	std::vector<Texture*> OpenGLFramebuffer::GetTextureAttachments() {
		return m_ColorAttachmentTex;
	}

	uint32_t OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y) {
		if (attachmentIndex >= m_ColorAttachmentTex.size()) {
			AHO_CORE_ERROR("Attachment index out of bound: {}", attachmentIndex);
			return 0u;
		}
		if (x >= m_Specification.Width || y >= m_Specification.Height) {
			AHO_CORE_ERROR("Attempting to read pixel data from an invalid position: {0}, {1}", x, y);
			return 0u;
		}
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		uint32_t pixelData;
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixelData);
		return pixelData;
	}

	// TODO: Mipmap level, filtering method
	void OpenGLFramebuffer::InvalidateColorAttachment() {
		for (size_t i = 0; i < m_ColorAttachmentSpecifications.size(); i++) {
			const auto& spec = m_ColorAttachmentSpecifications[i];
			uint32_t id;
			auto target = Utils::GetGLParam(spec.target);
			auto internalFormat = Utils::GetGLParam(spec.internalFormat);
			auto dataFormat = Utils::GetGLParam(spec.dataFormat);
			auto dataType = Utils::GetGLParam(spec.dataType);
			auto wrapModeS = Utils::GetGLParam(spec.wrapModeS);
			auto wrapModeT = Utils::GetGLParam(spec.wrapModeT);
			auto filterModeMin = Utils::GetGLParam(spec.filterModeMin);
			auto filterModeMag = Utils::GetGLParam(spec.filterModeMag);
			glCreateTextures(target, 1, &id);
			m_ColorAttachmentTex[i]->SetTextureID(id);
			glBindTexture(target, id);
			glTexImage2D(target, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, nullptr);
			if (wrapModeS) {
				glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModeS);
				glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModeT);
			}
			if (filterModeMag) {
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterModeMin);
				glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterModeMag);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, id, 0);
		}
	}
}