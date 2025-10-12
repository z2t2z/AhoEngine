#include "ContentBrowser.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/ResourceManager.h"
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

		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		{
			std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((m_FolderPath / "Asset" / "EngineAsset" / "Icons" / "arrow-left.png").string()));
			std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
			m_BackIcon = tex.get();
		}
		{
			std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((m_FolderPath / "Asset" / "EngineAsset" / "Icons" / "folder.png").string()));
			std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
			m_FolderIcon = tex.get();
		}
		{
			std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((m_FolderPath / "Asset" / "EngineAsset" / "Icons" / "file.png").string()));
			std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
			m_FileIcon = tex.get();
		}
	}

	void ContentBrowser::Draw() {
		// --- Constant settings ---
		constexpr float padding = 16.0f;
		constexpr float thumbnailSize = 80.0f;
		constexpr float cellSize = thumbnailSize + padding;
		constexpr float levelIconSize = 22.0f;
		constexpr ImVec4 disabledColor = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // grey tint when disabled

		ImGui::Begin(ICON_FA_FOLDER " Content Browser");
		
		// Back and forward buttons
		bool isRoot = m_CurrentPath == m_AssetPath;

		ImGui::BeginDisabled(isRoot);

		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::ImageButton("##back", (ImTextureID)m_BackIcon->GetTextureID(), ImVec2{ levelIconSize, levelIconSize }/*, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(0, 0, 0, 1)*/)) {
			if (!isRoot) // only navigate if allowed
				m_CurrentPath = m_CurrentPath.parent_path();
		}
		//ImGui::PopStyleVar();

		ImGui::EndDisabled(); // restore enabled state

		// Show current path text
		ImGui::SameLine();
		ImGui::TextUnformatted(m_CurrentPath.string().c_str());


		// Calculate how many columns we can fit
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = std::max(1, columnCount);

		ImGui::Separator();
		// Iterate over files and directories
		if (ImGui::BeginTable("ContentBrowserTable", columnCount)) {
			int index = 0;
			for (const auto& entry : fs::directory_iterator(m_CurrentPath)) {
				ImGui::TableNextColumn();

				std::string filename = entry.path().filename().string();
				const auto& path = entry.path();
				bool isDirectory = fs::is_directory(path);
				auto icon = isDirectory ? m_FolderIcon : m_FileIcon;

				ImGui::PushID(index++);

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton("Icon", (ImTextureID)icon->GetTextureID(), { thumbnailSize, thumbnailSize });
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					auto relativePath = entry.path().string();
					std::string identifier = "CONTENT_BROWSER_MESH";
					const static std::unordered_set<std::string> meshExtensions = { ".obj", ".fbx", ".gltf", ".glb" };
					if (!meshExtensions.count(entry.path().extension().string())) {
						identifier = "CONTENT_BROWSER_TEXTURE";
					}

					const char* itemPayload = relativePath.c_str();
					ImGui::SetDragDropPayload(identifier.c_str(), itemPayload, relativePath.length());
					ImGui::Text("File");
					ImGui::EndDragDropSource();
				}
				ImGui::PopStyleColor();

				// Handle double click for folders
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (isDirectory) {
						m_CurrentPath = path;
					}
				}

				// Center filename below icon
				ImGui::TextWrapped(filename.c_str());


				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		ImGui::End();
	}
}