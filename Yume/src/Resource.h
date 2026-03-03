#pragma once

#include "ResourceManager.h"

#include <string>

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

class Resource {
public:
    explicit Resource(const std::string& id) : m_resourceId(id) {}
    virtual ~Resource() = default;

    const std::string& GetId() const { return m_resourceId; }
    bool IsLoaded() const { return m_loaded; }

    virtual bool Load() {
        m_loaded = DoLoad();
        return m_loaded;
    }

    virtual void Unload() {
        DoUnload();
        m_loaded = false;
    }

protected:
    virtual bool DoLoad() = 0;
    virtual bool DoUnload() = 0;

private:
    std::string m_resourceId;
    bool m_loaded = false;
};

}