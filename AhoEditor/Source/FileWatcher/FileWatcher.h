#pragma once

#include <filesystem>

namespace Aho {
    // TODO: Support more file extensions
	class FileWatcher {
	public:
        FileWatcher() = default;
        bool HasFileContentChanged(const std::filesystem::path& FilePath) {
            std::time_t newModifiedTime = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
            if (newModifiedTime != m_FileChangedTime[FilePath]) {
                m_FileChangedTime[FilePath] = newModifiedTime;
                return true;
            }
            return false;
        }

        void AddFileToWatch(const std::filesystem::path& FilePath) {
            m_FileChangedTime[FilePath] = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
        }

        std::filesystem::path Poll() {
            for (const auto& [fileName, lstTime] : m_FileChangedTime) {
                if (HasFileContentChanged(fileName)) {
                    return fileName;
                }
            }
            return std::filesystem::path();
        }

	private:
        std::unordered_map<std::filesystem::path, std::time_t> m_FileChangedTime;
	};
} // namespace Aho
