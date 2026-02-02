#include "pch.h"
#include "Entity.h"
#include "components/Components.h"

Entity::Entity(entt::entity handle, entt::registry* registry)
    : _entityHandle(handle), _registry(registry)
{
}

TransformBundle Entity::GetTransformBundle()
{
    Transform* transform = nullptr;
    LocalToWorld* localToWorld = nullptr;
    
    // Get Transform component if it exists
    if (_registry->all_of<Transform>(_entityHandle))
    {
        transform = &_registry->get<Transform>(_entityHandle);
    }
    
    // Get LocalToWorld component if it exists
    if (_registry->all_of<LocalToWorld>(_entityHandle))
    {
        localToWorld = &_registry->get<LocalToWorld>(_entityHandle);
    }

    if (!transform || !localToWorld)
        return TransformBundle::Null();
    
    return TransformBundle(transform->Position, transform->Rotation, transform->Scale, localToWorld->Value);
}
