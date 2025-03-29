#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class ContentBrowser {
	public:
		ContentBrowser();
		void Initialize();
		void Draw();
	private:
		std::shared_ptr<Texture2D> m_BackIcon{ nullptr };
	private:
		std::filesystem::path m_FolderPath;
		std::filesystem::path m_AssetPath;
		std::filesystem::path m_CurrentPath;
	};
}
