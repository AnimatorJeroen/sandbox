#pragma once
#include <entt/entt.hpp>
#include <core/UUID.h>

/// <summary>
/// Entity is a wrapper around entt::entity that provides convenient component management.
/// It holds a reference to the scene's registry to enable component operations.
/// </summary>
class Entity
{
public:
    Entity() : _registry(nullptr), _entityHandle(entt::null) {}
    Entity(entt::entity handle, entt::registry* registry);
    Entity(const Entity& other) = default;

    /// <summary>
    /// Add a component to this entity
    /// </summary>
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        return _registry->emplace<T>(_entityHandle, std::forward<Args>(args)...);
    }

    /// <summary>
    /// Get a component from this entity
    /// </summary>
    template<typename T>
    T& GetComponent()
    {
        return _registry->get<T>(_entityHandle);
    }

    /// <summary>
    /// Get a component from this entity (const version)
    /// </summary>
    template<typename T>
    const T& GetComponent() const
    {
        return _registry->get<T>(_entityHandle);
    }

    /// <summary>
    /// Check if this entity has a component
    /// </summary>
    template<typename T>
    bool HasComponent() const
    {
        return _registry->all_of<T>(_entityHandle);
    }

    /// <summary>
    /// Get the UUID of this entity
    /// </summary>
    Core::UUID UUID() const
    {
        return GetComponent<Core::UUID>();
    }

    /// <summary>
    /// Get the underlying entt::entity handle
    /// </summary>
    entt::entity GetHandle() const { return _entityHandle; }

    /// <summary>
    /// Check if the entity is valid
    /// </summary>
    operator bool() const { return _entityHandle != entt::null && _registry != nullptr; }
    operator entt::entity() const { return _entityHandle; }

    bool operator==(const Entity& other) const
    {
        return _entityHandle == other._entityHandle && _registry == other._registry;
    }

    bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }

    bool operator<(const Entity& other) const
    {
        return _entityHandle < other._entityHandle;
    }

    /// <summary>
    /// Static null entity for comparisons
    /// </summary>
    static Entity Null() { return Entity(); }

private:
    entt::entity _entityHandle{ entt::null };
    entt::registry* _registry = nullptr;
};
