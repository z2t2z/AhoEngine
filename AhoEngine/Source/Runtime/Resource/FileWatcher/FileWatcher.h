#pragma once

#include <filesystem>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>

namespace Aho {
    class FileWatcher {
    public:
        using Callback = std::function<void()>;

        FileWatcher(std::chrono::duration<int, std::milli> interval = std::chrono::milliseconds(500))
            : watchInterval(interval), running(false) {
        }

        ~FileWatcher() {
            Stop();
        }

        // Watch a file path with a callback on change
        void WatchFile(const std::string& path, Callback cb) {
            auto time = std::filesystem::last_write_time(path);
            files[path] = { time, cb };
        }

        // Start watching in background thread
        void Start() {
            if (running) {
                return;
            }
            running = true;
            watcherThread = 
                std::thread([this]() {
                    while (running) {
                        for (auto& [path, info] : files) {
                            auto current = std::filesystem::last_write_time(path);
                            if (current != info.lastWrite) {
                                info.lastWrite = current;
                                info.callback();
                            }
                        }
                        std::this_thread::sleep_for(watchInterval);
                    }
                });
        }

        // Stop watching
        void Stop() {
            running = false;
            if (watcherThread.joinable()) watcherThread.join();
        }

    private:
        struct FileInfo {
            std::filesystem::file_time_type lastWrite;
            Callback callback;
        };

        std::unordered_map<std::string, FileInfo> files;
        std::chrono::duration<int, std::milli> watchInterval;
        std::thread watcherThread;
        std::atomic<bool> running;
    };
} // namespace Aho
