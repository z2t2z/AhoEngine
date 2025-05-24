#pragma once

#include "IamAho.h"
#include <memory>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class _Texture;
	class ContentBrowser {
	public:
		ContentBrowser();
		void Initialize();
		void Draw();
	private:
		_Texture* m_BackIcon{ nullptr };
	private:
		std::filesystem::path m_FolderPath;
		std::filesystem::path m_AssetPath;
		std::filesystem::path m_CurrentPath;
	};
}
