#pragma once

#include "Resource.h"

#include <unordered_map>
#include <memory>
#include <typeindex>

namespace Yume
{

template<typename T>
ResourceHandle<T> ResourceManager::Load(const std::string& resourceId) {
    static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

    auto& typeResources = m_resources[std::type_index(typeid(T))];
    auto it = typeResources.find(resourceId);

    if (it != typeResources.end()) {
        ++typeResources[resourceId].refCount;
        return ResourceHandle<T>(resourceId, this);
    }

    auto resource = std::make_shared<T>(resourceId);
    if (!resource->Load()) {
        return ResourceHandle<T>();
    }

    typeResources[resourceId].resource = resource;
    typeResources[resourceId].refCount = 1;

    return ResourceHandle<T>(resourceId, this);
}

template<typename T>
T* ResourceManager::GetResource(const std::string& resourceId) {
    auto typeIndex = std::type_index(typeid(T));

    if (m_resources.find(typeIndex) == m_resources.end()) {
        return nullptr;
    }

    auto& typeResources = m_resources[std::type_index(typeid(T))];

    auto it = typeResources.find(resourceId);
    if (it == typeResources.end()) {
        return nullptr;
    }

    return static_cast<T*>(it->second.get());
}

template<typename T>
bool ResourceManager::HasResource(const std::string& resourceId) {
    auto typeIndex = std::type_index(typeid(T));

    if (m_resources.find(typeIndex) == m_resources.end()) {
        return false;
    }

    auto& typeResources = m_resources[std::type_index(typeid(T))];
    return m_resources.find(typeIndex) != m_resources.end();
}

template<typename T>
void ResourceManager::Release(const std::string& resourceId) {
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

void ResourceManager::UnloadAll() {
    for (auto& [typeIndex, typeResources] : m_resources) {
        for (auto& [resourceId, resourceData] : typeResources) {
            resourceData.resource->Unload();
        }
        typeResources.clear();
    }
    m_resources.clear();
}

}