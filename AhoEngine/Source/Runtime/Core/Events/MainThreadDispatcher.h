#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <queue>

namespace Aho {
    class MainThreadDispatcher {
    public:
        using Task = std::function<void()>;

        static MainThreadDispatcher& Get() {
            static MainThreadDispatcher instance;
            return instance;
        }

        void Enqueue(Task task) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Tasks.emplace(std::move(task));
        }

        void Execute() {
            std::queue<Task> tasks;
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                std::swap(tasks, m_Tasks); 
            }
            while (!tasks.empty()) {
                tasks.front()();
                tasks.pop();
            }
        }

    private:
        std::mutex m_Mutex;
        std::queue<Task> m_Tasks;
    };
}
