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
    
    entt::entity parentEntity = entt::null;
    if (_registry->all_of<Parent>(_entityHandle))
    {
        auto parent = _registry->get<Parent>(_entityHandle);
        _registry->view<Core::UUID>().each([&](auto entity, const Core::UUID& uuid) {
            if (uuid.value == parent.parentUUID.value) {
                parentEntity = entity;
            }
        });
    }

    std::vector<entt::entity> children;
    if (_registry->all_of<Children>(_entityHandle))
    {
        auto childrenComp = _registry->get<Children>(_entityHandle);
		children = childrenComp.children;
    }

    return TransformBundle(transform->Position, transform->Rotation, transform->Scale,
        localToWorld->Value, parentEntity, children);
}
