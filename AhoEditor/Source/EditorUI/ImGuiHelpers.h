#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Aho {
	class ImGuiHelpers {
	public:
		/// Draw an image with rounded corners
		/// @param texID       The ImTextureID of the texture
		/// @param size        The size of the image to render (e.g., ImVec2(64,64))
		/// @param cornerR     The corner radius (e.g., 6.0f)
		/// @param uv0         Texture UV start (default 0,0)
		/// @param uv1         Texture UV end (default 1,1)
		/// @param tintCol     Tint color (default white)
		/// @param borderCol   Border color (optional)
		static void RoundedImage(ImTextureID texID, ImVec2 size, float cornerR = 6.0f, ImVec2 uv0 = ImVec2(0, 0), ImVec2 uv1 = ImVec2(1, 1), ImU32 tintCol = IM_COL32_WHITE, ImU32 borderCol = IM_COL32(255, 255, 255, 255));
		
		static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		
		static void DrawStatisticOverlay(bool* p_open, const std::vector<std::pair<std::string, float>>& infos);

	};
}
