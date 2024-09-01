#include "Ahopch.h"
#include "WindowsInput.h"

#include <GLFW/glfw3.h>
#include "Core/Application.h"

namespace Aho {
	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		//return std::make_pair((float)xpos, (float)ypos);
		return { (float)xpos, (float)ypos };
	}

	float WindowsInput::GetMouseXImpl() {
		auto [xpos, _] = WindowsInput::GetMousePositionImpl();
		return (float)xpos;
	}

	float WindowsInput::GetMouseYImpl() {
		auto [_, ypos] = WindowsInput::GetMousePositionImpl();
		return (float)ypos;
	}
	

}