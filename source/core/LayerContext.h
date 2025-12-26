#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Core
{
    class LayerContext {
    public:
        // Register a shared service by type T (e.g., ISimEngine, SimHost, Renderer, EventBus, …)
        template<typename T>
        void Register(std::shared_ptr<T> service) {
            _services[std::type_index(typeid(T))] = std::move(service);
        }

        // Retrieve a service by type T. Returns nullptr if not found.
        template<typename T>
        std::shared_ptr<T> Get() const {
            auto it = _services.find(std::type_index(typeid(T)));
            if (it == _services.end()) return {};
            return std::static_pointer_cast<T>(it->second);
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<void>> _services;
    };
}

