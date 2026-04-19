#pragma once

#include "Resource.h"
#include "ServiceLocator/ServiceLocator.h"
#include "Rendering/Resources/Shader.h"

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <filesystem>

namespace Yume
{

template<typename T>
class ResourceHandle;

class ResourceManager : public ServiceLocator::IService {
    inline static constexpr const char* ASSETS_DIRECTORY = "assets/";
    inline static constexpr const char* SHADERS_DIRECTORY = "shaders/";

    inline static std::unordered_map<std::type_index, std::string> m_assetTypeDirs = {
        { std::type_index(typeid(Shader)), SHADERS_DIRECTORY }
    };

public:
    virtual ~ResourceManager() = default;

    template<typename T>
    T* GetResource(const std::string& resourceId) {
        if (m_resources.find(std::type_index(typeid(T))) == m_resources.end()) {
            return nullptr;
        }

        auto& typeResources = m_resources[std::type_index(typeid(T))];

        auto it = typeResources.find(resourceId);
        if (it == typeResources.end()) {
            return nullptr;
        }

        return static_cast<T*>(it->second.resource.get());
    }

    template<typename T>
    bool HasResource(const std::string& resourceId) {
        auto typeIndex = std::type_index(typeid(T));

        if (m_resources.find(typeIndex) == m_resources.end()) {
            return false;
        }

        auto& typeResources = m_resources[typeIndex];
        return typeResources.find(resourceId) != typeResources.end();
    }

    template<typename T, typename... ResourceConstructionArgs>
    ResourceHandle<T> Load(const std::string& resourceId, ResourceConstructionArgs&&... resourceConstructionArgs) {
        return Load<T, ResourceConstructionArgs...>(resourceId, false, std::forward<ResourceConstructionArgs>(resourceConstructionArgs)...);
    }

    template<typename T>
    void Release(const std::string& resourceId) {
        auto typeIndex = std::type_index(typeid(T));

        if (m_resources.find(typeIndex) == m_resources.end()) {
            return;
        }

        auto& typeResources = m_resources[std::type_index(typeid(T))];

        auto it = typeResources.find(resourceId);
        if (it == typeResources.end()) {
            return;
        }

        auto& refCount = it->second.refCount;
        auto& resource = it->second.resource;

        --refCount;
        if (refCount <= 0) {
            resource->Unload();
            typeResources.erase(it);
        }
    }

    virtual void UnloadAll() {
        for (auto& [typeIndex, typeResources] : m_resources) {
            for (auto& [resourceId, resourceData] : typeResources) {
                resourceData.resource->Unload();
            }
            typeResources.clear();
        }
        m_resources.clear();
    }

protected:
    template<typename T, typename... ResourceConstructionArgs>
    ResourceHandle<T> Load(const std::string& resourceId, bool force_reload, ResourceConstructionArgs&&... resourceConstructionArgs) {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);

        if (it != typeResources.end()) {
            ++typeResources[resourceId].refCount;
            if (force_reload) {
                it->second.resource->Unload();
                it->second.resource->Load(GetAssetPath<T>(resourceId));
            }
            return ResourceHandle<T>(resourceId, this);
        }

        auto resource = std::make_unique<T>(resourceId, std::forward<ResourceConstructionArgs>(resourceConstructionArgs)...);
        if (!resource->Load(GetAssetPath<T>(resourceId))) {
            return ResourceHandle<T>();
        }

        auto& resourceData = typeResources[resourceId];
        resourceData.resource = std::move(resource);
        ++resourceData.refCount;

        return ResourceHandle<T>(resourceId, this);
    }

private:

    template<typename T>
    std::string GetAssetPath(const std::string& resourceId) {
        auto it = m_assetTypeDirs.find(std::type_index(typeid(T)));
        if (it != m_assetTypeDirs.end()) {
            auto assetPath = std::filesystem::absolute(ASSETS_DIRECTORY) / it->second / resourceId;
            return assetPath.string();
        } else {
            throw std::runtime_error("Unsupported asset type: " + std::string(typeid(T).name()));
        }
    }

private:
    struct ResourceData {
        std::unique_ptr<Resource> resource;
        int refCount = 0;
    };

    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> m_resources;
};

}