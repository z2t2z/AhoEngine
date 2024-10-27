#include "Ahopch.h"
#include "OpenGLFrameBuffer.h"
#include "Runtime/Core/Core.h"
#include <glm/glm.hpp>

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

		Texture* CreateNoiseTexture(int siz) {
			std::vector<glm::vec3> ssaoNoise;
			for (unsigned int i = 0; i < siz; i++) {
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
			FBTexture* noiseTex = new FBTexture(id);
			return noiseTex;
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
				case FBInterFormat::UINT:
					return GL_R32UI;
				case FBInterFormat::RED32F:
					return GL_R32F;
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
				case FBDataFormat::UINT:
					return GL_RED_INTEGER;
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
		for (size_t i = 0; i < m_Attchments.size(); i++) {
			m_ColorAttachmentTex.push_back(new FBTexture());
		}
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {
		if (m_DepthTex) {
			delete m_DepthTex;
		}
		for (Texture* tex : m_ColorAttachmentTex) {
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
		glCreateFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		// if has depth attachment, invalidate it
		if (m_DepthAttachmentSpecification.dataFormat == FBDataFormat::DepthComponent) {
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

	const std::vector<Texture*>& OpenGLFramebuffer::GetTextureAttachments() {
		return m_ColorAttachmentTex;
	}

	Texture* OpenGLFramebuffer::GetTextureAttachment(int index) {
		return m_ColorAttachmentTex[index];
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
		auto dataFormat = m_ColorAttachmentSpecifications[attachmentIndex].dataFormat;
		auto dataType = m_ColorAttachmentSpecifications[attachmentIndex].dataType;
		glReadPixels(x, y, 1, 1, Utils::GetGLParam(dataFormat), Utils::GetGLParam(dataType), &pixelData);
		AHO_CORE_WARN("{}", pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::InvalidateColorAttachment() {
		for (size_t i = 0; i < m_ColorAttachmentSpecifications.size(); i++) {
			auto& spec = m_ColorAttachmentSpecifications[i];
			uint32_t id;
			auto target = Utils::GetGLParam(spec.target);
			auto internalFormat = Utils::GetGLParam(spec.internalFormat);
			auto dataFormat = Utils::GetGLParam(spec.dataFormat);
			auto dataType = Utils::GetGLParam(spec.dataType);
			auto wrapModeS = Utils::GetGLParam(spec.wrapModeS);
			auto wrapModeT = Utils::GetGLParam(spec.wrapModeT);
			auto filterModeMin = Utils::GetGLParam(spec.filterModeMin);
			auto filterModeMag = Utils::GetGLParam(spec.filterModeMag);
			auto& mipLevels = spec.mipLevels;
			glCreateTextures(target, 1, &id);
			m_ColorAttachmentTex[i]->SetTextureID(id);
			auto& texSpec = m_ColorAttachmentTex[i]->GetSpecification();
			texSpec.Height = m_Specification.Height;
			texSpec.Width = m_Specification.Width;
			// TODO: this is too stupid. Texture class needs a big refactoring

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
			if (mipLevels) {
				spec.mipLevels = Utils::CalculateMaximumMipmapLevels(std::max(m_Specification.Height, m_Specification.Width));
				texSpec.mipmapLevels = spec.mipLevels;
				glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, spec.mipLevels);
				glGenerateMipmap(target);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, id, 0);
		}
	}
}