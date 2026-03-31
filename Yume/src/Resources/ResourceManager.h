#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <string>

namespace Yume
{

template<typename T>
class ResourceHandle;

class Resource;


class ResourceManager {
public:
    template<typename T>
    ResourceHandle<T> Load(const std::string& resourceId);

    template<typename T>
    T* GetResource(const std::string& resourceId);

    template<typename T>
    bool HasResource(const std::string& resourceId);

    template<typename T>
    void Release(const std::string& resourceId);

    void UnloadAll();

private:
    struct ResourceData {
        std::shared_ptr<Resource> resource;
        int refCount;
    };

    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> m_resources;
};

}