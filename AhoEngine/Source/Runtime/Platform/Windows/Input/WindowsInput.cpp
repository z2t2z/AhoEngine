#include "Ahopch.h"

#include "Runtime/Core/Input/Input.h"
#include "Runtime/Core/App/Application.h"

#include <GLFW/glfw3.h>

namespace Aho {

	bool Input::IsKeyPressed(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, keycode);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX() {
		auto [xpos, _] = Input::GetMousePosition();
		return (float)xpos;
	}

	float Input::GetMouseY() {
		auto [_, ypos] = Input::GetMousePosition();
		return (float)ypos;
	}
	
	void Input::LockCursor() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Input::SetCursorPos(float x, float y) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetCursorPos(window, x, y);
	}

	void Input::UnlockCursor() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool Input::GetCursorState() {
		return true;
	}

	bool Input::IsCursorLocked() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}
}