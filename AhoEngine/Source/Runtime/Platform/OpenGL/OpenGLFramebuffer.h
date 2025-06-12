#pragma once
#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/FrameBuffer.h"
#include <glad/glad.h>
#include <vector>

namespace Aho {
	enum class TextureUsage;
	class _Texture;
	class OpenGLFramebuffer : public Framebuffer {
	public:
		OpenGLFramebuffer(const std::vector<_Texture*>& cfgs, uint32_t width = 1600, uint32_t height = 900);
		OpenGLFramebuffer(const FBSpec& spec) {}
		virtual ~OpenGLFramebuffer();
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual bool Resize(uint32_t width, uint32_t height) override;
	private:
		uint32_t m_FBO{ 0u };
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
		FBSpec m_Specification;
	private:
		std::vector<_Texture*> m_Attachments;
		std::vector<GLenum> m_Attchments;
	};

}
