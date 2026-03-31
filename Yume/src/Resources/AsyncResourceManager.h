#pragma once

#include "ResourceManager.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

namespace Yume {

class AsyncResourceManager {
public:
    AsyncResourceManager() {
        Start();
    }

    ~AsyncResourceManager() {
        Stop();
    }

    void Start() {
        running = true;
        m_workerThread = std::thread([this]() {
            WorkerThread();
        });
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            running = false;
        }
        m_cv.notify_one();
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
    }

    template<typename T>
    void LoadAsync(const std::string& resourceId, std::function<void(ResourceHandle<T>)> callback) {
        std::lock_guard<std::mutex> lock(m_taskQueueMutex);
        m_taskQueue.push([this, resourceId, callback]() {
            auto handle = m_resourceManager.Load<T>(resourceId);
            callback(handle);
        });
        m_cv.notify_one();
    }

private:
    void WorkerThread() {
        while (running) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_taskQueueMutex);
                m_cv.wait(lock, [this]() {
                    return !m_taskQueue.empty() || !running;
                });

                if (!running && m_taskQueue.empty()) {
                    return;
                }

                task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
            }

            task();
        }
    }

private:
    ResourceManager m_resourceManager;

    std::thread m_workerThread;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex m_taskQueueMutex;
    std::condition_variable m_cv;

    bool running = false;
};

}