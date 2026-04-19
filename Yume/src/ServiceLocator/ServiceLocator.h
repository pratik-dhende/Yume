#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

namespace Yume {

class ServiceLocator {
public:
    class IService {
    public:
        virtual ~IService() = default;
    };

public:
    template<typename T>
    static void RegisterService(std::unique_ptr<T> service) {
        static_assert(std::is_base_of<IService, T>::value, "T must derive from IService");
        GetServices()[std::type_index(typeid(T))] = std::move(service);
    }

    template<typename T>
    static T& GetService() {
        static_assert(std::is_base_of<IService, T>::value, "T must derive from IService");
        auto it = GetServices().find(std::type_index(typeid(T)));
        if (it != GetServices().end()) {
            return static_cast<T&>(*it->second);
        }
        throw std::runtime_error("Service not found");
    }

private:
    static std::unordered_map<std::type_index, std::unique_ptr<IService>>& GetServices() {
        static std::unordered_map<std::type_index, std::unique_ptr<IService>> services;
        return services;
    }

};

}