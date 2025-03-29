#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <filesystem>
#include <vector>
#include <string>
 

namespace Aho {
    namespace fs = std::filesystem;
    class FileExplorer {
    public:
        static fs::path SelectFile(const fs::path& rootPath);
    };
}