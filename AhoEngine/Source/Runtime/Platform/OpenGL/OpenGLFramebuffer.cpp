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
				m_DepthTex = new OpenGLTexture2D();
				m_DepthAttachmentSpecification = spec;
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
		if (m_DepthAttachmentSpecification.dataFormat == TexDataFormat::DepthComponent) {
			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
			m_DepthTex->SetTextureID(m_DepthAttachment);
			glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
		}

		// if has color attachments, invalidate them
		if (!m_ColorAttachmentTex.empty()) {
			InvalidateColorAttachment();
			RebindSharedAttachments();
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

	void OpenGLFramebuffer::EnableAttachments(uint32_t start, uint32_t cnt) {
		if (cnt == 0) {
			cnt = m_Attchments.size();
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

	void OpenGLFramebuffer::BindSharedColorAttachment(Texture* attachment) {
		Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_ColorAttachmentTex.size() + m_SharedAttachmentTex.size(), GL_TEXTURE_2D, attachment->GetTextureID(), 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AHO_CORE_ASSERT(false, "Binding shared color attachment failed");
		}
		m_Attchments.push_back(GL_COLOR_ATTACHMENT0 + m_ColorAttachmentTex.size() + m_SharedAttachmentTex.size());
		m_SharedAttachmentTex.push_back(attachment);
		glDrawBuffers(static_cast<GLsizei>(m_Attchments.size()), m_Attchments.data());
		Unbind();
	}

	void OpenGLFramebuffer::BindSharedDepthAttachment(Texture* attachment) {
		Bind();
		m_SharedDepthTex = attachment;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachment->GetTextureID(), 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AHO_CORE_ASSERT(false, "Binding shared depth attachment failed");
		}
		Unbind();
	}

	void OpenGLFramebuffer::BindCubeMap(Texture* tex, int index, int attachmentID) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentID, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, tex->GetTextureID(), 0);
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

	uint32_t OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, bool shared) {
		const auto& readSource = shared ? m_SharedAttachmentTex : m_ColorAttachmentTex;
		if (attachmentIndex >= readSource.size()) {
			AHO_CORE_ERROR("Attachment index out of bound: {}", attachmentIndex);
			return 0u;
		}
		if (x >= m_Specification.Width || y >= m_Specification.Height) {
			AHO_CORE_ERROR("Attempting to read pixel data from an invalid position: {0}, {1}", x, y);
			return 0u;
		}
		uint32_t offset = shared ? m_ColorAttachmentTex.size() : 0;
		glReadBuffer(GL_COLOR_ATTACHMENT0 + offset + attachmentIndex);
		uint32_t pixelData;
		// TODO;;
		//auto dataFormat = readSource[attachmentIndex]->GetSpecification().dataFormat;
		//auto dataType = readSource.dataType;
		//glReadPixels(x, y, 1, 1, Utils::GetGLParam(dataFormat), Utils::GetGLParam(dataType), &pixelData);
		glReadPixels(x, y, 1, 1, Utils::GetGLParam(TexDataFormat::UINT), GL_UNSIGNED_BYTE, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::InvalidateColorAttachment() {
		for (size_t i = 0; i < m_ColorAttachmentTex.size(); i++) {
			m_ColorAttachmentTex[i]->GetSpecification().width = m_Specification.Width;
			m_ColorAttachmentTex[i]->GetSpecification().height = m_Specification.Height;
			m_ColorAttachmentTex[i]->Invalidate();
			auto target = Utils::GetGLParam(m_ColorAttachmentTex[i]->GetSpecification().target);
			auto id = m_ColorAttachmentTex[i]->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, id, 0);
		}
	}

	void OpenGLFramebuffer::RebindSharedAttachments() {
		uint32_t offset = m_ColorAttachmentTex.size();
		for (auto colorAttachment : m_SharedAttachmentTex) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + offset, GL_TEXTURE_2D, colorAttachment->GetTextureID(), 0);
			offset++;
		}
		if (m_SharedDepthTex) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_SharedDepthTex->GetTextureID(), 0);
		}
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AHO_CORE_ASSERT(false, "Binding shared depth attachment failed");
		}
	}
}