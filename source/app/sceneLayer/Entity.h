#pragma once
#include <entt/entt.hpp>
#include "core/Registry.h"
#include <core/UUID.h>

#include "TransformBundle.h"

/// <summary>
/// Entity is a wrapper around entt::entity that provides convenient component management.
/// It holds a reference to the scene's registry to enable component operations.
/// </summary>
class Entity
{
public:
    Entity() : _registry(nullptr), _entityHandle(entt::null) {}
    Entity(entt::entity handle, Core::Registry* registry);
    Entity(const Entity& other) = default;

    /// <summary>
    /// Add a component to this entity
    /// </summary>
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        return _registry->emplace<T>(_entityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    bool RemoveComponent()
    {
        if (HasComponent<T>())
        {
            _registry->remove<T>(_entityHandle);
            return true;
        }
        return false;
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
    /// Get transform info - helper to access all transform-related components
    /// </summary>
    TransformBundle GetTransformBundle();

	std::unordered_set<entt::entity> GetAllSiblingsIncludingSelf() const;
	std::unordered_set<entt::entity> GetAllSiblings() const;

    template<typename UntilComponentType>
    inline std::unordered_set<entt::entity> GetAllSiblingsUntilComponent() const
    {
        std::unordered_set<entt::entity> siblings;
        GetAllSiblingsRecursiveUntilComponent<UntilComponentType>(*this, siblings);
		return siblings;
    }

	Entity GetParent() const;

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

	void GetAllSiblingsRecursive(Entity parent, std::unordered_set<entt::entity>& siblings) const;

    template<typename UntilComponentType>
    inline void GetAllSiblingsRecursiveUntilComponent(Entity parent, std::unordered_set<entt::entity>& siblings) const
    {

        if (!parent.HasComponent<Children>())
            return;

        auto childrenComp = parent.GetComponent<Children>();
        for (entt::entity childHandle : childrenComp.children)
        {
            Entity childEntity(childHandle, _registry);
            if (childEntity.HasComponent<UntilComponentType>())
                continue;

            siblings.insert(childHandle);
            GetAllSiblingsRecursiveUntilComponent<UntilComponentType>(childEntity, siblings);
        }

    }

    entt::entity _entityHandle{ entt::null };
    Core::Registry* _registry = nullptr;
};
