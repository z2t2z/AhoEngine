#pragma once

#include "Runtime/Core/Events/Event.h"
#include "FileChangedEvent.h"
#include <filesystem>
#include <unordered_map>

namespace Aho {
    static uint32_t TIME_LAG = 2u;
    // TODO: Support more file extensions
	class FileWatcher {
        using FileChangedCallback = std::function<bool(FileChangedEvent&)>;
	public:
        FileWatcher() = default;
        void AddFileToWatch(const std::filesystem::path& FilePath) {
            m_FileChangedTime[FilePath] = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
        }

        void PollFiles() {
            for (const auto& [fileName, lstTime] : m_FileChangedTime) {
                if (IsFileContentChanged(fileName)) {
                    NotifyFileChanged(fileName.string(), fileName.string());
                }
            }
        }

        void SetCallback(FileChangedCallback fn) {
            m_Callback = fn;
        }

	private:
        void NotifyFileChanged(const std::string& filePath, const std::string& fileType) {
            FileChangedEvent event(filePath, fileType);
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<FileChangedEvent>(m_Callback);
        }

        bool IsFileContentChanged(const std::filesystem::path& FilePath) {
            std::time_t newModifiedTime = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
            if (newModifiedTime - m_FileChangedTime[FilePath] > TIME_LAG) {
                m_FileChangedTime[FilePath] = newModifiedTime;
                return true;
            }
            return false;
        }

    private:
        std::unordered_map<std::filesystem::path, std::time_t> m_FileChangedTime;
        FileChangedCallback m_Callback;
	};
} // namespace Aho
