#pragma once

#include "Core/Core.h"
#include "Core/Input/KeyCodes.h"
#include "Core/Input/MouseButtonCodes.h"


namespace Aho {

	class AHO_API Input {
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int keycode);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static void LockCursor();
		static void UnlockCursor();
		static bool GetCursorState();
		static bool IsCursorLocked();
	};

}