#pragma once

#include "ResourceManager.h"
#include "Texture.h"

#include <thread>
#include <filesystem>
#include <unordered_map>

namespace Yume {

// TODO: Only hot reload on refresh of engine.
class HotReloadResourceManager : public ResourceManager {
public:
    HotReloadResourceManager() {
        StartWatcher();
    }

    ~HotReloadResourceManager() {
        StopWatcher();
    }

    void StartWatcher() {
        m_running = true;
        m_watcherThread = std::thread([this]() {
            WatcherThread();
        });
    }

    void StopWatcher() {
        m_running = false;
        if (m_watcherThread.joinable()) {
            m_watcherThread.join();
        }
    }

    template<typename T>
    ResourceHandle<T> Load(const std::string& resourceId) {
        auto handle = ResourceManager::Load<T>(resourceId);

        std::string filePath = GetFilePath<T>(resourceId);
        try {
            m_fileTimestamps[filePath] = std::filesystem::last_write_time(filePath);
        } catch (const std::filesystem::filesystem_error& e) {
            // File doesn't exist or can't be accessed
        }

        return handle;
    }

private:
    template<typename T>
    std::string GetFilePath(const std::string& resourceId) {
        // Determine file path based on resource type and ID
        if constexpr (std::is_same_v<T, Texture>) {
            return "textures/" + resourceId + ".ktx";
        } else if constexpr (std::is_same_v<T, Mesh>) {
            return "models/" + resourceId + ".gltf";
        } else if constexpr (std::is_same_v<T, Shader>) {
            // Simplified for example
            return "shaders/" + resourceId + ".spv";
        } else {
            return "";
        }
    }

    void WatcherThread() {
        while (m_running) {
            for (auto& [filePath, timestamp] : m_fileTimestamps) {
                try {
                    auto currentTimestamp = std::filesystem::last_write_time(filePath);
                    if (currentTimestamp != timestamp) {
                        ReloadResource(filePath);
                        timestamp = currentTimestamp;
                    }
                } catch (const std::filesystem::filesystem_error& e) {
                    // File doesn't exist or can't be accessed
                }
            }

            // Sleep to avoid high CPU usage
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void ReloadResource(const std::string& filePath) {
        // Extract resource ID and type from file path
        // Reload the resource
        // ...
    }

private:
    std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps;
    std::thread m_watcherThread;
    bool m_running = false;
};
}