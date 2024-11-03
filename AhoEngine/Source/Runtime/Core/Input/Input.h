#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Input/KeyCodes.h"
#include "Runtime/Core/Input/MouseButtonCodes.h"


namespace Aho {

	class Input {
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int keycode);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static void LockCursor();
		static void UnlockCursor();
		static bool GetCursorState();
		static void SetCursorPos(float x, float y);
		static bool IsCursorLocked();
	};

}