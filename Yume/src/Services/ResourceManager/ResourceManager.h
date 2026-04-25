#pragma once

#include "Resource.h"
#include "ServiceLocator/ServiceLocator.h"
#include "Rendering/Resources/Shader.h"
#include "Rendering/Resources/Texture.h"

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <stb_image.h>

namespace Yume
{

template<typename T>
class ResourceHandle;

class ResourceManager : public ServiceLocator::IService {
    using Path = std::filesystem::path;

protected:
    inline static constexpr const char* ASSETS_DIRECTORY = "assets/";
    inline static constexpr const char* SHADERS_DIRECTORY = "shaders/";
    inline static constexpr const char* TEXTURES_DIRECTORY = "textures/";

    inline static std::unordered_map<std::type_index, std::string> m_assetTypeDirs = {
        { std::type_index(typeid(Shader)), SHADERS_DIRECTORY },
        { std::type_index(typeid(Texture)), TEXTURES_DIRECTORY }
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
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = m_resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);

        if (it != typeResources.end()) {
            ++typeResources[resourceId].refCount;
            return ResourceHandle<T>(resourceId, this);
        }

        auto resource = std::make_unique<T>(resourceId, std::forward<ResourceConstructionArgs>(resourceConstructionArgs)...);
        if (!resource->Load()) {
            return ResourceHandle<T>();
        }

        auto& resourceData = typeResources[resourceId];
        resourceData.resource = std::move(resource);
        ++resourceData.refCount;

        return ResourceHandle<T>(resourceId, this);
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

    template<typename T>
    bool ReadFile(const std::string& resourceId, std::vector<char>& buffer) {
        auto filePath = GetNormalizedAssetPath<T>(resourceId).string();

        std::ifstream file(filePath, std::ios::ate);

        if (!file.is_open()) {
            throw std::runtime_error(filePath);
        }

        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

        file.close();

        return true;
    }

    template<typename T>
    bool ReadImage(const std::string& resourceId, int& outImageWidth, int& outImageHeight, int& outImageChannels, stbi_uc** out) {
        auto filePath = GetNormalizedAssetPath<T>(resourceId).string();
        int imageWidth, imageHeight, imageChannels;
        auto pixels = stbi_load(filePath.c_str(), &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        outImageWidth = imageWidth;
        outImageHeight = imageHeight;
        outImageChannels = imageChannels;
        *out = pixels;

        return true;
    }

    template<typename T>
    std::string GetAssetPath(const std::string& resourceId) {
        return GetNormalizedAssetPath<T>(resourceId).generic_string();
    }


protected:
    struct ResourceData {
        std::unique_ptr<Resource> resource;
        int refCount = 0;
    };

    template<typename T>
    ResourceData& GetResourceData(const std::string& resourceId) {
        auto typeIndex = std::type_index(typeid(T));

        if (m_resources.find(typeIndex) == m_resources.end()) {
            throw std::runtime_error("Resource type not found: " + std::string(typeid(T).name()));
        }

        auto& typeResources = m_resources[typeIndex];
        auto it = typeResources.find(resourceId);
        if (it == typeResources.end()) {
            throw std::runtime_error("Resource not found: " + resourceId);
        }

        return it->second;
    }

    template<typename T>
    Path GetNormalizedAssetPath(const std::string& resourceId) {
        auto it = m_assetTypeDirs.find(std::type_index(typeid(T)));
        if (it != m_assetTypeDirs.end()) {
            return ToNormalizedPath(ASSETS_DIRECTORY + it->second + resourceId);
        }
        else {
            throw std::runtime_error("Unsupported asset type: " + std::string(typeid(T).name()));
        }
    }

    static Path ToNormalizedPath(const std::string& filePath) {
        return std::filesystem::absolute(filePath).lexically_normal();
    }

    static auto SubPath(const Path& path, const Path& subPath) {
        return std::search(path.begin(), path.end(), subPath.begin(), subPath.end());
    }

private:
    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> m_resources;
};

}