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
        transform = &_registry->get<Transform>(_entityHandle);

    // Get LocalToWorld component if it exists
    if (_registry->all_of<LocalToWorld>(_entityHandle))
        localToWorld = &_registry->get<LocalToWorld>(_entityHandle);

    if (!transform || !localToWorld)
        return TransformBundle::Null();
    
    Parent* parent = nullptr;
    if (_registry->all_of<Parent>(_entityHandle))
        parent = &_registry->get<Parent>(_entityHandle);

    Children* children = nullptr;
    if (_registry->all_of<Children>(_entityHandle))
        children = &_registry->get<Children>(_entityHandle);


    return TransformBundle(transform->Position, transform->Rotation, transform->Scale, 
        localToWorld->Value, parent ? parent->parentUUID : Core::UUID::Null, children ? children->children : std::vector<entt::entity>());
}
