#include "pch.h"
#include "Entity.h"

Entity::Entity(entt::entity handle, entt::registry* registry)
    : _entityHandle(handle), _registry(registry)
{
}
