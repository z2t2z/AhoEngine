#pragma once

#include "Core/Core.h"
#include "Core/Input/KeyCodes.h"
#include "Core/Input/MouseButtonCodes.h"


namespace Aho {

	class AHO_API Input {
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int keycode) { return s_Instance->IsMouseButtonPressedImpl(keycode); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int keycode) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;
	};

}