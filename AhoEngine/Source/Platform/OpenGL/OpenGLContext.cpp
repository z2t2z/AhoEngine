#include "Ahopch.h"
#include "OpenGLContext.h"
#include "Core/Core.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Aho {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		AHO_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init() {
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AHO_CORE_ASSERT(status, "Failed to initialize Glad!");

		/*AHO_CORE_INFO("OpenGL Info:");
		AHO_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		AHO_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		AHO_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));*/

	}

	void OpenGLContext::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}

}