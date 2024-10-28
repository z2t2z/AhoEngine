#include "Ahopch.h"
#include "OpenGLContext.h"
#include "Runtime/Core/Core.h"

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
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//glEnable(GL_CULL_FACE);    
		//glCullFace(GL_BACK);       
		//glFrontFace(GL_CCW);       
		//GLint maxUniformBlockSize;
		//glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
		//AHO_CORE_INFO("{}", maxUniformBlockSize);

		//AHO_CORE_INFO("OpenGL Info:");
		//AHO_CORE_INFO("Vendor: {0}", glGetString(GL_VENDOR));
		//AHO_CORE_INFO("Renderer: {0}", glGetString(GL_RENDERER));
		//AHO_CORE_INFO("Version: " + glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}

}