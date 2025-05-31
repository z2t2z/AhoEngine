#pragma once

#include <imgui.h>
#include <imgui_internal.h>

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
		static void RoundedImage(ImTextureID texID, ImVec2 size, float cornerR = 6.0f, ImVec2 uv0 = ImVec2(0, 0), ImVec2 uv1 = ImVec2(1, 1),
			ImU32 tintCol = IM_COL32_WHITE,
			ImU32 borderCol = IM_COL32(255, 255, 255, 255)) {
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			drawList->AddImageRounded(texID, pos, ImVec2(pos.x + size.x, pos.y + size.y),
				uv0, uv1, tintCol, cornerR);

			if (borderCol & IM_COL32_A_MASK) {
				drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderCol, cornerR);
			}

			ImGui::Dummy(size); // Reserve space
		}

		static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			bool modified = false;

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize)) {
				modified = true;
				values.x = resetValue;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize)) {
				modified = true;
				values.y = resetValue;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize)) {
				modified = true;
				values.z = resetValue;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();

			return modified;
		}

	};
}
