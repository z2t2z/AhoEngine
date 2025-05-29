#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <iostream>
#include <queue>

namespace Aho {
    class ParallelExecutor {
    public:
        ParallelExecutor(int numThreads = std::thread::hardware_concurrency()) : stop(false) {
            if (numThreads <= 0) numThreads = 1;
            for (int i = 0; i < numThreads; ++i) {
                workers.emplace_back([this] { this->workerThread(); });
            }
        }

        ~ParallelExecutor() {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                stop = true;
            }
            condition.notify_all();
            for (auto& worker : workers) {
                if (worker.joinable()) worker.join();
            }
        }

        void enqueue(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                tasks.push(std::move(task));
            }
            condition.notify_one();
        }

        void ParallelFor(int64_t count, std::function<void(int64_t)> func, int chunkSize = 64) {
            // 使用shared_ptr包裹同步变量，保证生命周期
            auto nextIndex = std::make_shared<std::atomic<int64_t>>(0);
            auto activeWorkers = std::make_shared<std::atomic<int>>(0);
            auto finishMutex = std::make_shared<std::mutex>();
            auto finishCondition = std::make_shared<std::condition_variable>();

            int numThreads = (int)workers.size();
            if (numThreads == 0) numThreads = 1;

            // 启动任务
            for (int i = 0; i < numThreads; ++i) {
                auto task = [=]() {
                    activeWorkers->fetch_add(1, std::memory_order_relaxed);
                    while (true) {
                        int64_t start = nextIndex->fetch_add(chunkSize, std::memory_order_relaxed);
                        if (start >= count) break;
                        int64_t end = std::min(start + chunkSize, count);
                        for (int64_t j = start; j < end; ++j) {
                            func(j);
                        }
                    }
                    if (activeWorkers->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                        std::lock_guard<std::mutex> lock(*finishMutex);
                        finishCondition->notify_one();
                    }
                    };
                enqueue(std::move(task));
            }

            // 等待所有任务完成
            std::unique_lock<std::mutex> lock(*finishMutex);
            finishCondition->wait(lock, [&] { return activeWorkers->load(std::memory_order_acquire) == 0; });
        }

    private:
        // 工作线程主循环
        void workerThread() {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();
            }
        }

        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop;
    };
}