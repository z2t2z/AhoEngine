#include "ContentBrowser.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include <filesystem>

namespace Aho {
	namespace fs = std::filesystem;

	ContentBrowser::ContentBrowser() {
	}

	void ContentBrowser::Initialize() {
		m_FolderPath = fs::current_path();
		m_AssetPath = m_FolderPath / "Asset";
		m_CurrentPath = m_AssetPath;
		m_BackIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "back.png").string());
	}

	void ContentBrowser::Draw() {
		ImGui::Begin(ICON_FA_FOLDER " Content Browser");
		if (m_CurrentPath != m_AssetPath) {
			if (ImGui::ImageButton("##back", ImTextureID(m_BackIcon->GetTextureID()), ImVec2{ 18, 18 })) {
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}
		for (const auto& entry : fs::directory_iterator(m_CurrentPath)) {
			auto fileName = entry.path().filename().string();
			if (ImGui::Button(fileName.c_str())) {
				if (fs::is_directory(entry)) {
					m_CurrentPath = entry.path();
				}
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				auto relativePath = entry.path().string();
				std::string identifier = "CONTENT_BROWSER_MESH";
				if (entry.path().extension() != ".obj" && entry.path().extension() != ".fbx") {
					identifier = "CONTENT_BROWSER_TEXTURE";
				}
				const char* itemPayload = relativePath.c_str();
				ImGui::SetDragDropPayload(identifier.c_str(), itemPayload, relativePath.length());
				ImGui::Text("File");
				ImGui::EndDragDropSource();
			}
		}
		ImGui::End();
	}
}
