#include "Ahopch.h"
#include "OpenGLTexture.h"
#include "OpenGLFrameBuffer.h"
#include "Runtime/Core/Core.h"
#include <glm/glm.hpp>

namespace Aho {
	static const uint32_t s_MaxFramebufferSize = 8192;

	OpenGLFramebuffer::OpenGLFramebuffer(const FBSpec& spec)
		: m_Specification(spec) {
		for (const auto& spec : m_Specification.Attachments) {
			if (!Utils::IsDepthFormat(spec.dataFormat)) {
				m_ColorAttachmentTex.push_back(new OpenGLTexture2D(spec));
				m_Attchments.push_back(GL_COLOR_ATTACHMENT0 + m_Attchments.size());
			}
			else {
				m_DepthTex = new OpenGLTexture2D(spec);
			}
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

	void OpenGLFramebuffer::Invalidate() {
		if (m_FBO) {
			glDeleteFramebuffers(1, &m_FBO);
		}
		glCreateFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

		// If has depth attachment, invalidate it
		if (m_DepthTex) {
			m_DepthTex->GetSpecification().width = m_Specification.Width;
			m_DepthTex->GetSpecification().height = m_Specification.Height;
			m_DepthTex->Invalidate();
			auto target = Utils::GetGLParam(m_DepthTex->GetSpecification().target);
			auto id = m_DepthTex->GetTextureID();
			auto xx = Utils::GetGLParam(m_DepthTex->GetSpecification().dataFormat) == GL_DEPTH_COMPONENT ? GL_DEPTH_ATTACHMENT : GL_DEPTH_STENCIL_ATTACHMENT;
			glFramebufferTexture2D(GL_FRAMEBUFFER, xx, target, id, 0);

			GLenum err = glGetError();
			if (err != GL_NO_ERROR) {
				AHO_CORE_ASSERT(false);
			}

		}

		// if has color attachments, invalidate them
		if (!m_ColorAttachmentTex.empty()) {
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
		glDrawBuffers(static_cast<GLsizei>(m_Attchments.size()), m_Attchments.data());
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// temp
	void OpenGLFramebuffer::BindAt(uint32_t pos, uint32_t colorAttachmentId) {
		glBindImageTexture(pos, m_ColorAttachmentTex.at(colorAttachmentId)->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}

	bool OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
		if (!m_ResizeWithViewport) {
			return false;
		}

		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
			AHO_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return false;
		}

		if (m_Specification.Width == width && m_Specification.Height == height) {
			return false;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
		return true;
	}

	void OpenGLFramebuffer::EnableAttachments(uint32_t start, uint32_t cnt) {
		if (cnt == 0) {
			cnt = m_Attchments.size() - start;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
		if (cnt == 0) {
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else {
			glDrawBuffers(cnt, m_Attchments.data() + start);
		}
	}

	void OpenGLFramebuffer::BindCubeMap(Texture* tex, int faceIndex, int attachmentID, int mipLevel) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentID, GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, tex->GetTextureID(), mipLevel);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AHO_CORE_ASSERT(false, "Binding shared depth attachment failed");
		}
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

	Texture* OpenGLFramebuffer::GetTexture(TexType type) {
		auto it = std::find_if(m_ColorAttachmentTex.begin(), m_ColorAttachmentTex.end(), 
			[&](Texture* tex) {
				return tex->GetTexType() == type;
			});
		AHO_CORE_ASSERT(it != m_ColorAttachmentTex.end());
		return *it;
	}

	uint32_t OpenGLFramebuffer::ReadPixel(TexType type, uint32_t x, uint32_t y, bool shared) {
		auto it = std::find_if(m_ColorAttachmentTex.begin(), m_ColorAttachmentTex.end(), [&](Texture* tex) {
			return tex->GetTexType() == type;
		});
		AHO_CORE_ASSERT(it != m_ColorAttachmentTex.end(), "Did not find this attachment");
		if (x >= m_Specification.Width || y >= m_Specification.Height) {
			AHO_CORE_ERROR("Attempting to read pixel data from an invalid position: {0}, {1}", x, y);
			return 0u;
		}
		glReadBuffer(GL_COLOR_ATTACHMENT0 + it - m_ColorAttachmentTex.begin());
		uint32_t pixelData;
		// TODO: how to read float value?
		glReadPixels(x, y, 1, 1, Utils::GetGLParam(TexDataFormat::UINT), GL_UNSIGNED_INT, &pixelData);
		//AHO_CORE_WARN("Reading pixel data: {}, {}, {}", x, y, pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::InvalidateColorAttachment() {
		for (size_t i = 0; i < m_ColorAttachmentTex.size(); i++) {
			m_ColorAttachmentTex[i]->GetSpecification().width = m_Specification.Width;
			m_ColorAttachmentTex[i]->GetSpecification().height = m_Specification.Height;
			m_ColorAttachmentTex[i]->Invalidate();
			auto target = Utils::GetGLParam(m_ColorAttachmentTex[i]->GetSpecification().target);
			auto id = m_ColorAttachmentTex[i]->GetTextureID();

			if (target == GL_TEXTURE_CUBE_MAP) {
				for (int face = 0; face < 6; face++) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, id, 0);
				}
			}
			else if (target == GL_TEXTURE_2D) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, id, 0);
			}
			else {
				AHO_CORE_ASSERT(false);
			}
		}
	}
}