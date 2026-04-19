#pragma once

#include <string>
#include "ResourceManager.h"

namespace Yume {

template<typename T>
class ResourceHandle {
public:
    ResourceHandle() : m_resourceManager(nullptr) {}

    ResourceHandle(const std::string& id, ResourceManager* manager)
        : m_resourceId(id), m_resourceManager(manager) {
    }

    T* Get() const {
        if (!m_resourceManager) return nullptr;
        return m_resourceManager->GetResource<T>(m_resourceId);
    }

    bool IsValid() const {
        return m_resourceManager && m_resourceManager->HasResource<T>(m_resourceId);
    }

    const std::string& GetId() const {
        return m_resourceId;
    }

    T* operator->() const {
        return Get();
    }

    T& operator*() const {
        return *Get();
    }

    operator bool() const {
        return IsValid();
    }

private:
    std::string m_resourceId;
    ResourceManager* m_resourceManager;
};

}