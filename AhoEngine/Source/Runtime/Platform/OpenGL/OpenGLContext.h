#pragma once

#include "Runtime/Function/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Aho {

	class OpenGLContext : public GraphicsContext {
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};


}
