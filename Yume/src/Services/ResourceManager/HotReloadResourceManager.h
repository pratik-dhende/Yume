#pragma once

#include "ResourceManager.h"
#include "Rendering/Resources/Texture.h"

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
        {
            std::lock_guard<std::mutex> lock(m_runningMutex);
            m_running = true;
        }
        m_watcherThread = std::thread([this]() {
            WatcherThread();
        });
    }

    void StopWatcher() {
        {
            std::lock_guard<std::mutex> lock(m_runningMutex);
            m_running = false;
        }
        if (m_watcherThread.joinable()) {
            m_watcherThread.join();
        }
    }

    void HotReload() {
        std::unordered_set<std::string> filesToReloadCopy;
        {
            std::lock_guard<std::mutex> lock(m_reloadMutex);
            filesToReloadCopy = m_filesToReload;
        }
        for (const auto& filePath : filesToReloadCopy) {
            ReloadResource(filePath);
        }
    }

    template<typename T>
    T* GetResource(const std::string& resourceId) override{
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        return ResourceManager::GetResource<T>(resourceId);
    }

    template<typename T>
    bool HasResource(const std::string& resourceId) override {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        return ResourceManager::HasResource<T>(resourceId);
    }

    template<typename T, typename... ResourceConstructionArgs>
    ResourceHandle<T> Load(const std::string& resourceId, ResourceConstructionArgs&&... resourceConstructionArgs) override{
        if (!HasResource<T>(resourceId)) {
            std::string filePath = GetAssetPath<T>(resourceId);
            try {
                std::lock_guard<std::mutex> lock(m_fileStatesMutex);
                m_fileStates[filePath] = std::filesystem::last_write_time(filePath);
            } catch (const std::filesystem::filesystem_error& e) {
                throw std::runtime_error("Failed to get file timestamp for: " + filePath);
            }
        }
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        auto handle = ResourceManager::Load<T>(resourceId, false, std::forward<ResourceConstructionArgs>(resourceConstructionArgs)...);
        return handle;
    }

    template<typename T>
    void Release(const std::string& resourceId) override{
        {
            std::lock_guard<std::mutex> lock(m_resourcesMutex);
            ResourceManager::Release<T>(resourceId);
        }
        UnwatchResource<T>(resourceId);
    }

    void UnloadAll() override{
        {
            std::lock_guard<std::mutex> lock(m_resourcesMutex);
            ResourceManager::UnloadAll();
        }
        {
            std::lock_guard<std::mutex> lock(m_fileStatesMutex);
            m_fileStates.clear();
        }
        {
            std::lock_guard<std::mutex> lock(m_reloadMutex);
            m_filesToReload.clear();
        }
    }

private:
    void WatcherThread() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(m_runningMutex);
                if (!m_running) {
                    break;
                }
            }
            {
                std::lock_guard<std::mutex> lock(m_fileStatesMutex);

                for (auto& [filePath, lastWriteTimestamp] : m_fileStates) {
                    try {
                        auto currentTimestamp = std::filesystem::last_write_time(filePath);

                        if (currentTimestamp != lastWriteTimestamp) {
                            {
                                std::lock_guard<std::mutex> lock(m_reloadMutex);
                                m_filesToReload.insert(filePath);
                            }
                            lastWriteTimestamp = currentTimestamp;
                        }
                    } catch (const std::filesystem::filesystem_error& e) {
                        // File doesn't exist or can't be accessed
                    }
                }
            }
            // Sleep to avoid high CPU usage
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void GetAssetTypeAndResourceFromFilepath(const std::string& filePath, std::type_index& outType, std::string& outResourceId) {
        for (const auto& [typeIndex, typeDir] : m_assetTypeDirs) {
            if (filePath.find(ASSETS_DIRECTORY + typeDir) == 0) {
                outType = typeIndex;
                outResourceId = filePath.substr((ASSETS_DIRECTORY + typeDir).length());
                return;
            }
        }
        throw std::runtime_error("Failed to determine asset type from file path: " + filePath);
    }

    void ReloadResource(const std::string& filePath) {
        std::type_index assetType = typeid(void);
        std::string resourceId;
        GetAssetTypeAndResourceFromFilepath(filePath, assetType, resourceId);
        
        if (assetType == std::type_index(typeid(Shader))) {
            std::lock_guard<std::mutex> lock(m_resourcesMutex);
            ResourceManager::Load<Shader>(resourceId, true);
        } else {
            throw std::runtime_error("Unsupported asset type for reloading: " + std::string(assetType.name()));
        }
    }

    template<typename T>
    void UnwatchResource(const std::string& resourceId) {
        std::string filePath = GetAssetPath<T>(resourceId);
        {
            std::lock_guard<std::mutex> lock(m_fileStatesMutex);
            m_fileStates.erase(filePath);
        }
        {
            std::lock_guard<std::mutex> reloadLock(m_reloadMutex);
            m_filesToReload.erase(filePath);
        }
    }

private:
    std::unordered_map<std::string, std::filesystem::file_time_type> m_fileStates;
    std::unordered_set<std::string> m_filesToReload;

    std::mutex m_reloadMutex;
    std::mutex m_runningMutex;
    std::mutex m_fileStatesMutex;
    std::mutex m_resourcesMutex;

    std::thread m_watcherThread;
    bool m_running = false;
};
}