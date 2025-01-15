#include "Ahopch.h"
#include "OpenGLContext.h"
#include "Runtime/Core/Core.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Aho {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		AHO_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar* message, const void* userParam) {

		AHO_CORE_WARN("\nGL Debug Message:\nSource: {} \nType: {} \nID: {} \nSeverity: {} \nMessage: {}\n",
			source, type, id, severity, message);

	}

	void OpenGLContext::Init() {
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AHO_CORE_ASSERT(status, "Failed to initialize Glad!");  

		glEnable(GL_DEBUG_OUTPUT); // 启用调试输出
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // 确保调试回调在错误发生时立即调用

		glDebugMessageCallback(DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

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