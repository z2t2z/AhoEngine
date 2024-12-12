#pragma once

#include "Runtime/Core/Events/Event.h"
#include "Runtime/Core/SingletonBase.h"
#include "FileChangedEvent.h"
#include <filesystem>
#include <unordered_map>

namespace Aho {

    // TODO: supports multi-thread
    // upd: hard to support mt currently since opengl natively doesn't support it
    class FileWatcher : public SingletonBase<FileWatcher> {
        friend class SingletonBase<FileWatcher>;
	public:
        ~FileWatcher() = default;

        template <typename T>
        void AddFileToWatch_mt(const std::filesystem::path& filePath, const std::shared_ptr<T>& file) {
            if (m_Files.contains(filePath)) {
                return;
            }

            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Files[filePath] = std::make_shared<FileRecord<T>>(file, std::filesystem::last_write_time(filePath));
        }

        template <typename T>
        void AddFileToWatch(const std::filesystem::path& filePath, const std::shared_ptr<T>& file) {
            if (m_Files.contains(filePath)) {
                return;
            }

            m_Files[filePath] = std::make_shared<FileRecord<T>>(file, std::filesystem::last_write_time(filePath));
        }

        void StartWatching() {
            m_Running = true;
            m_WatchThread = std::thread(&FileWatcher::PollFiles_mt, this);
        }
        
        void StopWatching_mt() {
            m_Running = false;
            if (m_WatchThread.joinable()) {
                m_WatchThread.join();
            }
        }

        void PollFiles_mt() {
            static int64_t sleepTimeLag = 3; // check files every 3 seconds
            while (m_Running) {
                std::this_thread::sleep_for(std::chrono::seconds(sleepTimeLag));
                std::lock_guard<std::mutex> lock(m_Mutex);

                for (auto& [filePath, fileRecord] : m_Files) {
                    auto newModifiedTime = std::filesystem::last_write_time(filePath);
                    if (fileRecord->lastWriteTime != newModifiedTime) {
                        fileRecord->Reload(filePath);
                        fileRecord->lastWriteTime = newModifiedTime;
                    }
                }

            }
        }

        // in seconds
        void PollFiles(float deltaTime) {
            m_AccuTime += deltaTime;
            if (m_AccuTime > 3.0f) {
                m_AccuTime = 0.0f;
                for (auto& [filePath, fileRecord] : m_Files) {
                    auto newModifiedTime = std::filesystem::last_write_time(filePath);
                    if (fileRecord->lastWriteTime != newModifiedTime) {
                        fileRecord->Reload(filePath);
                        fileRecord->lastWriteTime = newModifiedTime;
                    }
                }
            }
        }

	private:
        FileWatcher() = default;
        float m_AccuTime{ 0.0f };

    private:
        std::thread m_WatchThread;
        std::atomic<bool> m_Running{ false };
        std::mutex m_Mutex;

    private:
        struct IFileRecord {
            IFileRecord(const std::filesystem::file_time_type& time) : lastWriteTime(time) {}
            virtual ~IFileRecord() = default;
            virtual void Reload(const std::filesystem::path& path) = 0;
            std::filesystem::file_time_type lastWriteTime;
        };

        template <typename T>
        struct FileRecord : public IFileRecord {
            FileRecord(const std::shared_ptr<T>& f, const std::filesystem::file_time_type& time) 
                : IFileRecord(time), file(f) {}
            std::shared_ptr<T> file;

            virtual void Reload(const std::filesystem::path& path) override {
                file->Reload(path);
            }
        };

        std::unordered_map<std::filesystem::path, std::shared_ptr<IFileRecord>> m_Files;
	};
} // namespace Aho
