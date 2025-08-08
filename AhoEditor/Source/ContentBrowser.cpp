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
		std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((m_FolderPath / "Asset" / "EngineAsset" / "Icons" / "plusicon.png").string()));
		std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
		m_BackIcon = tex.get();
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
				const static std::unordered_set<std::string> meshExtensions = { ".obj", ".fbx", ".gltf", ".glb" };
				/*if (entry.path().extension() != ".obj" && entry.path().extension() != ".fbx") {
					identifier = "CONTENT_BROWSER_TEXTURE";
				}*/
				if (!meshExtensions.count(entry.path().extension().string())) {
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
