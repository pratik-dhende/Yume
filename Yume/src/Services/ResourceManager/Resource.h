#pragma once

#include <string>

namespace Yume {
    
class Resource {
public:
    explicit Resource(const std::string& id) : m_resourceId(id) {}
    virtual ~Resource() = default;

    const std::string& GetId() const { return m_resourceId; }
    bool IsLoaded() const { return m_loaded; }

    bool Load(const std::string& filepath) {
        m_loaded = DoLoad(filepath);
        return m_loaded;
    }

    bool Unload() {
        m_loaded = !DoUnload();
        return !m_loaded;
    }

protected:
    virtual bool DoLoad(const std::string& filepath) = 0;
    virtual bool DoUnload() = 0;

private:
    std::string m_resourceId;
    bool m_loaded = false;
};

}