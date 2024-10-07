#pragma once

#include <filesystem>

namespace Aho {
    // TODO: Support more file extensions
    const static float s_Period = 1.0f;
	class FileWatcher {
	public:
        FileWatcher() = default;
        void AddFileToWatch(const std::filesystem::path& FilePath) {
            m_FileChangedTime[FilePath] = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
        }

        std::filesystem::path Poll(float deltaTime) {
            if (!Check(deltaTime)) {
                return std::filesystem::path();
            }
            for (const auto& [fileName, lstTime] : m_FileChangedTime) {
                if (IsFileContentChanged(fileName)) {
                    return fileName;
                }
            }
            return std::filesystem::path();
        }

	private:
        bool IsFileContentChanged(const std::filesystem::path& FilePath) {
            std::time_t newModifiedTime = std::filesystem::last_write_time(FilePath).time_since_epoch().count();
            if (newModifiedTime != m_FileChangedTime[FilePath]) {
                m_FileChangedTime[FilePath] = newModifiedTime;
                return true;
            }
            return false;
        }

        bool Check(float deltaTime) {
            m_AccumulativeTime += deltaTime;
            if (m_AccumulativeTime > s_Period) {
                m_AccumulativeTime = 0.0f;
                return true;
            }
            return false;
        }

        std::unordered_map<std::filesystem::path, std::time_t> m_FileChangedTime;
        float m_AccumulativeTime{ 0.0f };
	};
} // namespace Aho
