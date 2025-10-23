#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include "Runtime/Function/Renderer/RenderSettings.h"

namespace Aho {
	class RenderSettingsPenal {
	public:
		RenderSettingsPenal() = default;
		void Draw();
	private:
		void ShowShadowSettingsPanel();
	private:
		RenderSettings m_Settings;
		ShadowSettings m_ShadowSettings;
	};
}