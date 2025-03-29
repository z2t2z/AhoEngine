#include "FileExplorer.h"

namespace Aho {
    fs::path FileExplorer::SelectFile(const fs::path& root_path) {
        //static std::string selected_path;
        static fs::path selected_path;
        static fs::path current_path = root_path;
        static std::vector<fs::path> back_stack;
        static std::vector<fs::path> forward_stack;

        // 开始文件浏览器窗口
        ImGui::Begin("File Browserff", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize);


        // 导航控制栏
        ImGui::BeginDisabled(back_stack.empty());
        if (ImGui::Button("< Back") && !back_stack.empty()) {
            forward_stack.push_back(current_path);
            current_path = back_stack.back();
            back_stack.pop_back();
            selected_path.clear();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text("Current: %s", current_path.string().c_str());

        // 文件列表
        if (ImGui::BeginChild("FileList", ImVec2(400, 300), true)) {
            try {
                // 列出目录内容
                for (const auto& entry : fs::directory_iterator(current_path)) {
                    const bool is_directory = entry.is_directory();
                    const bool is_selected = (selected_path == entry.path());

                    // 显示目录/文件项
                        ImGui::PushID(entry.path().string().c_str());
                        if (is_directory) {
                            if (ImGui::Selectable("[D] ", false,
                                ImGuiSelectableFlags_AllowDoubleClick)) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    back_stack.push_back(current_path);
                                    current_path = entry.path();
                                    forward_stack.clear();
                                    selected_path.clear();
                                }
                            }
                        }
                        else {
                            if (ImGui::Selectable("[F] ", is_selected,
                                ImGuiSelectableFlags_AllowDoubleClick)) {
                                selected_path = entry.path();
                            }
                        }
                    ImGui::SameLine();
                    ImGui::Text("%s", entry.path().filename().string().c_str());
                    ImGui::PopID();
                }
            }
            catch (const fs::filesystem_error&) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Cannot access directory");
            }
        }
        ImGui::EndChild();

        // 操作按钮
        bool done = false;
        ImGui::BeginDisabled(selected_path.empty());
        if (ImGui::Button("Select")) {
            done = true;
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            selected_path.clear();
            done = true;
        }

        ImGui::End();

        if (done) {
            // 重置状态
            current_path = root_path;
            back_stack.clear();
            forward_stack.clear();
            return std::move(selected_path);
        }
        return "";
	}
}
