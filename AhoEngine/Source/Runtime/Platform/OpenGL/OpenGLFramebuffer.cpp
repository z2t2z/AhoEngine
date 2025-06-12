#include "Ahopch.h"
#include "OpenGLTexture.h"
#include "OpenGLFrameBuffer.h"
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include <glm/glm.hpp>

namespace Aho {
	static const uint32_t s_MaxFramebufferSize = 8192;

	OpenGLFramebuffer::OpenGLFramebuffer(const std::vector<_Texture*>& cfgs, uint32_t width, uint32_t height)
	: m_Attachments(cfgs) {
		Resize(width, height);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer() {
		if (m_FBO) {
			glDeleteFramebuffers(1, &m_FBO);
			m_FBO = 0;
		}
	}

	bool OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
		if (!m_FBO) {
			glCreateFramebuffers(1, &m_FBO);
		}
		if (m_Width == width && m_Height == height) {
			return false;
		}

		m_Width = width; m_Height = height;
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		uint32_t offset = 0;
		std::vector<uint32_t> drawBuffers; drawBuffers.reserve(m_Attachments.size());
		for (auto& attachment : m_Attachments) {
			attachment->Resize(width, height);
			uint32_t fmt = (uint32_t)attachment->GetDataFmt();
			uint32_t type;
			if (fmt == DataFormat::Depth || fmt == DataFormat::DepthStencil) {
				type = fmt == DataFormat::Depth ? GL_DEPTH_ATTACHMENT : GL_DEPTH_STENCIL_ATTACHMENT;
			}
			else {
				type = GL_COLOR_ATTACHMENT0 + offset++;
				drawBuffers.push_back(type);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, attachment->GetDim(), attachment->GetTextureID(), 0);
		}
		if (drawBuffers.empty()) 
			glDrawBuffer(GL_NONE);
		else 
			glDrawBuffers(drawBuffers.size(), drawBuffers.data());

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AHO_CORE_ASSERT(false, "Framebuffer is incomplete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	void OpenGLFramebuffer::Bind() {
		int width = m_Attachments[0]->GetWidth(), height = m_Attachments[0]->GetHeight();
		if (m_Width != width || m_Height != height) {
			Resize(width, height);
		}
		AHO_CORE_ASSERT(m_Width == width, m_Height == height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glViewport(0, 0, width, height);
	}

	void OpenGLFramebuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}